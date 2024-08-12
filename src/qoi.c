/*
 * @brief This file helps read image from m24m02
 */
#include "qoi.h"
#include "common.h"

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(qoi, LOG_LEVEL_DBG);
/*
 * @brief PNG (Portable Network Graphics) makes up with file signature
 *        and at least 3 chunk (IHDR, IDAT and IEND). 
 *        Chunk can be classified as Critical Chunk and Ancillary Chunk, 
 *        in which Critical Chunk is the nessary chunk and Ancillary Chunk is optional. 
 *        Structure: 
 *        (1) file signature - 8 byte data, 0x89 0x50 0x4E 0x47 0x0D 0x0A 0x1A 0x0A
 *        (2) IHDR chunk (Image Header chunk) - first chunk, contains basic info, 13 byte, 
 *        image width (4 byte), image height (4 byte), bit depth (1 byte), color type (1 byte), 
 *        compression method (1 byte), filter method (1 byte), interlace method (1 byte). 
 *        (3) PLTE chunk (Palette chunk), should be in front of IDAT if exist. 
 *        (4) IDAT chunk (Image Data chunk). 
 *        (5) IEND chunk (Image Trailer chunk). 
 *        E.G. 
 *        Here is a 4 * 4 png image - 
 *        +-----+-----+-----+-----+
 *        | red | red |green|green|
 *        +-----+-----+-----+-----+
 *        | red | red |green|green|
 *        +-----+-----+-----+-----+
 *        |white|white|blue |blue |
 *        +-----+-----+-----+-----+
 *        |white|white|blue |blue |
 *        +-----+-----+-----+-----+
 *        Hex file - 
 *        89 50 4e 47 0d 0a 1a 0a | 00 00 00 0d | 49 48 44 52 | 00 00 00 04 | 00 00 00 04 |
 *        08 | 02 | 00 | 00 | 00 | 26 93 09 29 | 00 00 00 01 | 73 52 47 42 | 00 | ae ce 1c e9 |
 *        00 00 00 04 | 67 41 4d 41 | 00 00 b1 8f | 0b fc 61 05 | 00 00 00 09 | 70 48 59 73 |
 *        00 00 0e c3 | 00 00 0e c3 | c7 6f a8 64 | 00 00 00 1e | 49 44 41 54 | 18 57 63 78 2b 
 *        a3 02 44 4a 1b 7d 80 08 95 f3 1f 0c 18 18 c0 08 c1 61 f8 0f 00 4c e2 19 21 | 8f e2 45 a6 |
 *        00 00 00 00 | 49 45 4e 44 | ae 42 60 82
 *        Analysis - 
 *        89 50 4e 47 0d 0a 1a 0a - file signature
 *        00 00 00 0d - next chunk length, 13 byte
 *        49 48 44 52 - ASCII "IHDR" 
 *        00 00 00 04 - IHDR chunk, width
 *        00 00 00 04 - IHDR chunk, height
 *        08 - IHDR chunk, bit depth is 8 bit
 *        02 - IHDR chunk, color type is truecolor
 *        00 - IHDR chunk, compression method, fixed
 *        00 - IHDR chunk, filter method, fixed
 *        00 - IHDR chunk, non interlaced scanning
 *        26 93 09 29 - IHDR chunk, CRC
 *        00 00 00 01 - next chunk length, 1 byte
 *        73 52 47 42 - ASCII "sRGB" (Standard RGB color space chunk)
 *        00 - sRGB chunk, perceptual
 *        ae ce 1c e9 - sRGB chunk, CRC
 *        00 00 00 04 - next chunk length, 4 byte
 *        67 41 4d 41 - ASCII "gAMA" (Image gamma chunk)
 *        00 00 b1 8f - gAMA chunk, fixed
 *        0b fc 61 05 - gAMA chunk, CRC
 *        00 00 00 09 - next chunk length, 9 byte
 *        70 48 59 73 - ASCII "pHYs" (Physical pixel dimensions chunk)
 *        00 00 0e c3 - pHYs chunk, pixels per unit, X axis, 3779 pixel per meter
 *        00 00 0e c3 - pHYs chunk, pixels per unit, Y axis, 3779 pixel per meter
 *        01 - pHYs chunk, meter
 *        c7 6f a8 64 - pHYs chunk, CRC
 *        00 00 00 1e - next chunk length, 30 byte
 *        49 44 41 54 - ASCII "IDAT"
 *        18 ... 21 - IDAT chunk, data
 *        8f e2 45 a6 - IDAT chunk, CRC
 *        00 00 00 00 - next chunk length, 0 byte
 *        49 45 4e 44 - ASCII "IEND"
 *        ae 42 60 82 - IEND chunk, CRC
 *        Now we want to focus on the IDAT chunk. 
 *        Original image -> Filtering -> LZSS -> Huffman Coding -> png
 *        Check: 
 *        https://www.bilibili.com/video/BV1wY4y1P7o7/?vd_source=8ed6e4b33f1dedf514ff24f19d1e27c7
 * 
 *        Now, we want to talk about the QOI (Quite OK Image format). 
 *        A QOI file consists of a 14-byte header, followed by any number of data chunks and 
 *        an 8-byte end marker. 
 *        qoi_header {
 *            char     magic[4];    // magic bytes "qoif"
 *            uint32_t width;       // image width in pixels (BE)
 *            uint32_t height;      // image height in pixels (BE)
 *            uint8_t  channels;    // 3 = RGB, 4 = RGBA
 *            uint8_t  colorspace;  // 0 = sRGB with linear alpha, 1 = all channels linear
 *        };
 *        The channel and colorspace fields are purely informative. 
 *        Images are encoded row by row, left to right, top to bottom. The 
 *        decoder and encoder start with {r: 0, g: 0, b: 0, a: 255} as the 
 *        previous pixel value.
 *        Chunk rules: 
 *        (1) if the current pixel is the same value as the previous pixel, 
 *        QOI encodes it using a run length and will increment the run length until it sees a 
 *        different pixel. 
 *        E.G. 
 *                  pre. 
 *        R ch -    255     255     255     255     255     59
 *        G ch -    255     255     255     255     255     8
 *        B ch -    92      92      92      92      92      147
 *        +-------------------------------+
 *        |            Byte[0]            |
 *        | 7   6   5   4   3   2   1   0 |
 *        +-------------------------------+
 *        | 1   1 |           4           |
 *        +-------------------------------+
 *        (2) Storing the rgb value of the current pixel as a difference from the previous pixel. 
 *        if the difference between the previously seen pixel value and the current pixel value
 *        is within a predefined constraint (-2 <= dr <= 1, -2 <= dg <= 1, -2 <= db <= 1), 
 *        we can store the current rgb pixel in a single byte of memory. 
 *        E.G.
 *                  pre. 
 *        R ch -    140     138
 *        G ch -    77      78
 *        B ch -    251     251
 *        +-------------------------------+
 *        |            Byte[0]            |
 *        | 7   6   5   4   3   2   1   0 |
 *        +-------------------------------+
 *        | 0   1 |  -2   |   1   |   0   |
 *        +-------------------------------+
 *        (3) QOI also has an option to encode larger differences in two bytes of memory. 
 *        if the previous pixel and current pixel differences meet the following criteria
 *        (-32 <= dg <= 31, -8 <= dr - dg <= 7, -8 <= db - dg <= 7), it is possible to encode the 
 *        current pixel in two bytes. 
 *        E.G. 
 *                  pre. 
 *        R ch -    140     142
 *        G ch -    77      72
 *        B ch -    252     251
 *        dg = 72 - 77 = -5
 *        dr - dg = (142 - 140) - (72 - 77) = 7
 *        db - dg = (251 - 252) - (72 - 77) = 4
 *        +-------------------------------+-------------------------------+
 *        |            Byte[0]            |            Byte[1]            |
 *        | 7   6   5   4   3   2   1   0 | 7   6   5   4   3   2   1   0 |
 *        +-------------------------------+-------------------------------+
 *        | 1   0 |           -5          |       7       |       4       |
 *        +-------------------------------+-------------------------------+
 *        (4) Encoding a pixel as an index of a previously seen pixel. 
 *        As QOI processes pixels one by one, it'll store each pixel in an array. 
 *        We determine the location to place the process pixel by using a rather simple 
 *        hash of the r, g and b pixel value. QOI uses an array of size 64 which which allows 
 *        the index location to be stored in 6 bits. 
 *        E.G.
 *        +-----------------------------------------------------------+
 *        |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
 *        +-----------------------------------------------------------+
 *                      140                                       140
 *                      77                                        77
 *                      251                                       252
 *        R ch -    ...     140
 *        G ch -    ...     77
 *        B ch -    ...     251
 *        +-------------------------------+
 *        |            Byte[0]            |
 *        | 7   6   5   4   3   2   1   0 |
 *        +-------------------------------+
 *        | 0   0 |           2           |
 *        +-------------------------------+
 *        (5) If Run Length Encoding (RLE) is not possible, the pixel has never been encountered 
 *        in our array, and the difference of a pixel is too large, we'll just store the rgb value 
 *        as is in memory. 
 *        +----------------+----------------+----------------+----------------+
 *        |    Byte[0]     |    Byte[1]     |    Byte[2]     |    Byte[3]     |
 *        |  7   ...   0   |  7   ...   0   |  7   ...   0   |  7   ...   0   |
 *        +----------------+----------------+----------------+----------------+
 *        | 8 bit RGB_TAG  |      Red       |     Green      |      Blue      |
 *        +----------------+----------------+----------------+----------------+
 *        Let's have a try: 
 *        E.G. 
 *        R ch -    140     255     255     255     59      140     140     255
 *        G ch -    77      255     255     255     8       77      77      255
 *        B ch -    251     92      92      92      147     252     251     92
 *        +----------------+----------------+----------------+----------------+
 *        |    Byte[0]     |    Byte[1]     |    Byte[2]     |    Byte[3]     |
 *        |  7   ...   0   |  7   ...   0   |  7   ...   0   |  7   ...   0   |
 *        +----------------+----------------+----------------+----------------+
 *        | 8 bit RGB_TAG  |      140       |      77        |      251       |
 *        +----------------+----------------+----------------+----------------+
 * 
 *        +----------------+----------------+----------------+----------------+
 *        |    Byte[4]     |    Byte[5]     |    Byte[6]     |    Byte[7]     |
 *        |  7   ...   0   |  7   ...   0   |  7   ...   0   |  7   ...   0   |
 *        +----------------+----------------+----------------+----------------+
 *        | 8 bit RGB_TAG  |      255       |      255       |       92       |
 *        +----------------+----------------+----------------+----------------+
 * 
 *        +-------------------------------+
 *        |            Byte[8]            |
 *        | 7   6   5   4   3   2   1   0 |
 *        +-------------------------------+
 *        | 1   1 |           2           |
 *        +-------------------------------+
 * 
 *        +----------------+----------------+----------------+----------------+
 *        |    Byte[9]     |    Byte[10]    |    Byte[11]    |    Byte[12]    |
 *        |  7   ...   0   |  7   ...   0   |  7   ...   0   |  7   ...   0   |
 *        +----------------+----------------+----------------+----------------+
 *        | 8 bit RGB_TAG  |       59       |        8       |       147      |
 *        +----------------+----------------+----------------+----------------+
 * 
 *        +----------------+----------------+----------------+----------------+
 *        |    Byte[13]    |    Byte[14]    |    Byte[15]    |    Byte[16]    |
 *        |  7   ...   0   |  7   ...   0   |  7   ...   0   |  7   ...   0   |
 *        +----------------+----------------+----------------+----------------+
 *        | 8 bit RGB_TAG  |      140       |       77       |       252      |
 *        +----------------+----------------+----------------+----------------+
 * 
 *        +-------------------------------+
 *        |           Byte[17]            |
 *        | 7   6   5   4   3   2   1   0 |
 *        +-------------------------------+
 *        | 0   0 |           2           |
 *        +-------------------------------+
 * 
 *        +-------------------------------+
 *        |           Byte[18]            |
 *        | 7   6   5   4   3   2   1   0 |
 *        +-------------------------------+
 *        | 0   0 |           60          |
 *        +-------------------------------+
 *  
 *        We compress 24 byte into 18 byte. 
 * 
 *        My adding rules: 
 *        (1) index array[0] is for alpha = 0 pixel. 
 */

/*
 * @brief qoi test func
 */
void qoi_init(void) {
    
}

/*
 * @brief QOI encode func
 *
 * @param raw_buf data to be encoded
 * @param qoi_buf buffer to store encoded data
 * @param length raw data length
 *
 * @retval qoi data length
 * 
 * @warning overflow may occurs
 */
static size_t qoi_encode(uint8_t *raw_buf, uint8_t *qoi_buf, size_t length) {
    size_t raw_buf_shift = 0;
    size_t qoi_buf_shift = 0;
    size_t count_0x00 = 0;
    bool flag_0x00 = false;
    while (1) {
        if (raw_buf_shift + 1 > length) {
            if (flag_0x00 == true) {                                                                // should write final 0x00 sequence into qoi_buf
                uint8_t count_255 = count_0x00 / 255;
                uint8_t remain = count_0x00 % 255;
                for (uint8_t i = 0; i < count_255; i++) {
                    *(qoi_buf + qoi_buf_shift) = 0x00;
                    qoi_buf_shift++;
                    *(qoi_buf + qoi_buf_shift) = 0xFF;
                    qoi_buf_shift++;
                }
                if (remain != 0) {
                    *(qoi_buf + qoi_buf_shift) = 0x00;
                    qoi_buf_shift++;
                    *(qoi_buf + qoi_buf_shift) = remain;
                    qoi_buf_shift++;
                }
                count_0x00 = 0;
                flag_0x00 = false;

                return qoi_buf_shift;
            } else {
                return qoi_buf_shift;
            }
        }

        if (*(raw_buf + raw_buf_shift) != 0x00 && flag_0x00 == false) {                             // meet !0x00 and no 0x00 before
            *(qoi_buf + qoi_buf_shift) = *(raw_buf + raw_buf_shift);
            raw_buf_shift++;
            qoi_buf_shift++;
        } else if (*(raw_buf + raw_buf_shift) != 0x00 && flag_0x00 == true) {                       // meet !0x00 and settlement 0x00 before
            uint8_t count_255 = count_0x00 / 255;
            uint8_t remain = count_0x00 % 255;
            for (uint8_t i = 0; i < count_255; i++) {
                *(qoi_buf + qoi_buf_shift) = 0x00;
                qoi_buf_shift++;
                *(qoi_buf + qoi_buf_shift) = 0xFF;
                qoi_buf_shift++;
            }
            if (remain != 0) {
                *(qoi_buf + qoi_buf_shift) = 0x00;
                qoi_buf_shift++;
                *(qoi_buf + qoi_buf_shift) = remain;
                qoi_buf_shift++;
            }
            count_0x00 = 0;
            flag_0x00 = false;
            *(qoi_buf + qoi_buf_shift) = *(raw_buf + raw_buf_shift);
            raw_buf_shift++;
            qoi_buf_shift++;
        } else {                                                                                    // meet 0x00
            count_0x00++;
            flag_0x00 = true;
            raw_buf_shift++;
        }
    }
}

/*
 * @brief QOI decode func
 *
 * @param qoi_buf data to be decoded
 * @param raw_buf buffer to store decoded data
 * @param length raw data length
 *
 * @retval qoi data length
 */
static size_t qoi_decode(uint8_t *qoi_buf, uint8_t *raw_buf, size_t length) {
    size_t qoi_buf_shift = 0;
    size_t raw_buf_shift = 0;
    while (1) {
        if (qoi_buf_shift + 1> length) {
            return raw_buf_shift;
        }
        if (*(qoi_buf + qoi_buf_shift) != 0x00) {
            *(raw_buf + raw_buf_shift) = *(qoi_buf + qoi_buf_shift);
            qoi_buf_shift++;
            raw_buf_shift++;
        } else {
            for (int i = 0; i < *(qoi_buf + qoi_buf_shift + 1); i++) {
                *(raw_buf + raw_buf_shift) = 0x00;
                raw_buf_shift++;
            }
            qoi_buf_shift += 2;
        }
    }
}

/*
 * @brief 
 *
 * @param 
 *
 * @retval
 */

