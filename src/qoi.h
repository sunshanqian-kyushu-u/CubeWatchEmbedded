#ifndef _QOI_H_
#define _QOI_H_

#include <zephyr/kernel.h>

struct image_signature_st {
    uint8_t signature[3];
};

struct image_chunk_st {
    uint16_t length;
    uint8_t magic[3];
    uint8_t *data;
};

// uint8_t test_raw_image_header_chunk_data[] = {
//         0x04, // width
//         0x04, // height
//         0x08  // channel, 0x03 = RGB888, 0x04 = RGB888A8, 0x05 = RGB444, 
//               // 0x06 = RGB565, 0x07 = RGB666, 0x08 = A1RGB555, 0x09 = my QOI
// };

// uint8_t test_raw_image_data_chunk_data[] = {
//         0xFC, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 
//         0x00, 0x00, 0x00, 0x00, 0x83, 0xE0, 0x83, 0xE0, 
//         0x00, 0x00, 0x00, 0x00, 0x80, 0x1F, 0x80, 0x1F, 
//         0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

// struct image_signature_st test_raw_image_signature = {
//     .signature = {0x} // ASCII "raw"
// };

// struct image_chunk_st test_raw_image_header_chunk = {
//     .length = sizeof(test_raw_image_header_chunk_data) + 3, 
//     .magic = "hdr", 
//     .data = test_raw_image_header_chunk_data
// };

// struct image_chunk_st test_raw_image_data_chunk = {
//     .length = sizeof(test_raw_image_data_chunk_data) + 3,
//     .magic = "dat", 
//     .data = test_raw_image_data_chunk_data
// };

// uint8_t test_qoi_image[100];

static uint8_t raw_data[] = {
        /*
         * id - 0x00, num - 0x00
         */
        0x72, 0x61, 0x77,                                                                           // ASCII "raw"

        0x00, 0x02,                                                                                 // uuid chunk - length
        0x75, 0x69, 0x64,                                                                           // uuid chunk - ASCII "uid"
        0x00, 0x00, 

        0x00, 0x03,                                                                                 // header chunk - length
        0x68, 0x64, 0x72,                                                                           // header chunk - ASCII "hdr"
        0x04,                                                                                       // header chunk - width
        0x04,                                                                                       // header chunk - height
        0x08,                                                                                       // header chunk - channel

        0x00, 0x20,                                                                                 // data chunk - length
        0x64, 0x61, 0x74,                                                                           // data chunk - ASCII "dat"
        /*
        * +-----+-----+-----+-----+
        * | red | red |trans|trans|
        * +-----+-----+-----+-----+
        * |trans|trans|green|green|
        * +-----+-----+-----+-----+
        * |trans|trans|blue |blue |
        * +-----+-----+-----+-----+
        * |white|white|trans|trans|
        * +-----+-----+-----+-----+
        * 
        * red - b1111110000000000 = 0xFC, 0x00
        * green - b1000001111100000 = 0x83, 0xE0
        * blue - b1000000000011111 = 0x80, 0x1F
        * white - b1111111111111111 = 0xFF, 0xFF
        * trans = transparent - b0000000000000000 = 0x00, 0x00
        */
        0xFC, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "red", "red", "trans", "trans"
        0x00, 0x00, 0x00, 0x00, 0x83, 0xE0, 0x83, 0xE0,                                             // data chunk - "trans", "trans", "green", "green"
        0x00, 0x00, 0x00, 0x00, 0x80, 0x1F, 0x80, 0x1F,                                             // data chunk - "trans", "trans", "blue", "blue"
        0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "white", "white", "trans", "trans"

        0x00, 0x00,                                                                                 // end chunk - length
        0x65, 0x6E, 0x64,                                                                           // end chunk - ASCII "end"

        /*
         * id - 0x00, num - 0x01
         */
        0x72, 0x61, 0x77,                                                                           // ASCII "raw"

        0x00, 0x02,                                                                                 // uuid chunk - length
        0x75, 0x69, 0x64,                                                                           // uuid chunk - ASCII "uid"
        0x00, 0x01, 

        0x00, 0x03,                                                                                 // header chunk - length
        0x68, 0x64, 0x72,                                                                           // header chunk - ASCII "hdr"
        0x04,                                                                                       // header chunk - width
        0x04,                                                                                       // header chunk - height
        0x08,                                                                                       // header chunk - channel

        0x00, 0x80,                                                                                 // data chunk - length
        0x64, 0x61, 0x74,                                                                           // data chunk - ASCII "dat"
        /*
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        * |trans|trans|trans|trans|trans|trans|trans|trans|
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        * |trans|trans|trans|trans|trans|trans|trans|trans|
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        * |trans|trans| red | red |white|white|trans|trans|
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        * |trans|trans| red | red |white|white|trans|trans|
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        * |trans|trans|blue |blue |green|green|trans|trans|
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        * |trans|trans|blue |blue |green|green|trans|trans|
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        * |trans|trans|trans|trans|trans|trans|trans|trans|
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        * |trans|trans|trans|trans|trans|trans|trans|trans|
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        * 
        * red - b1111110000000000 = 0xFC, 0x00
        * green - b1000001111100000 = 0x83, 0xE0
        * blue - b1000000000011111 = 0x80, 0x1F
        * white - b1111111111111111 = 0xFF, 0xFF
        * trans = transparent - b0000000000000000 = 0x00, 0x00
        */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "trans", "trans", "trans", "trans"
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "trans", "trans", "trans", "trans"

        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "trans", "trans", "trans", "trans"
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "trans", "trans", "trans", "trans"

        0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0xFC, 0x00,                                             // data chunk - "trans", "trans", "red", "red"
        0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "white", "white", "trans", "trans"

        0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0xFC, 0x00,                                             // data chunk - "trans", "trans", "red", "red"
        0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "white", "white", "trans", "trans"

        0x00, 0x00, 0x00, 0x00, 0x80, 0x1F, 0x80, 0x1F,                                             // data chunk - "trans", "trans", "blue", "blue"
        0x83, 0xE0, 0x83, 0xE0, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "green", "green", "trans", "trans"

        0x00, 0x00, 0x00, 0x00, 0x80, 0x1F, 0x80, 0x1F,                                             // data chunk - "trans", "trans", "blue", "blue"
        0x83, 0xE0, 0x83, 0xE0, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "green", "green", "trans", "trans"

        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "trans", "trans", "trans", "trans"
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "trans", "trans", "trans", "trans"

        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "trans", "trans", "trans", "trans"
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                             // data chunk - "trans", "trans", "trans", "trans"

        0x00, 0x00,                                                                                 // end chunk - length
        0x65, 0x6E, 0x64,                                                                           // end chunk - ASCII "end"

        /*
         * id - 0x00, num - 0x02
         */
        0x72, 0x61, 0x77,                                                                           // ASCII "raw"

        0x00, 0x02,                                                                                 // uuid chunk - length
        0x75, 0x69, 0x64,                                                                           // uuid chunk - ASCII "uid"
        0x00, 0x02, 

        0x00, 0x03,                                                                                 // header chunk - length
        0x68, 0x64, 0x72,                                                                           // header chunk - ASCII "hdr"
        0x04,                                                                                       // header chunk - width
        0x04,                                                                                       // header chunk - height
        0x08,                                                                                       // header chunk - channel

        0x00, 0x48,                                                                                 // data chunk - length
        0x64, 0x61, 0x74,                                                                           // data chunk - ASCII "dat"
        /*
        * +-----+-----+-----+-----+-----+-----+
        * |white|white|white|trans|trans|trans|
        * +-----+-----+-----+-----+-----+-----+
        * |trans|trans|trans| red | red | red |
        * +-----+-----+-----+-----+-----+-----+
        * |green|green|green|trans|trans|trans|
        * +-----+-----+-----+-----+-----+-----+
        * |trans|trans|trans|blue |blue |blue |
        * +-----+-----+-----+-----+-----+-----+
        * | red |green|blue |white|trans| red |
        * +-----+-----+-----+-----+-----+-----+
        * |green|blue |white|trans|green|green|
        * +-----+-----+-----+-----+-----+-----+
        * 
        * red - b1111110000000000 = 0xFC, 0x00
        * green - b1000001111100000 = 0x83, 0xE0
        * blue - b1000000000011111 = 0x80, 0x1F
        * white - b1111111111111111 = 0xFF, 0xFF
        * trans = transparent - b0000000000000000 = 0x00, 0x00
        */
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                                                         // data chunk - "white", "white", "white"
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                                         // data chunk - "trans", "trans", "trans"

        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                                         // data chunk - "trans", "trans", "trans"
        0xFC, 0x00, 0xFC, 0x00, 0xFC, 0x00,                                                         // data chunk - "red", "red", "red"

        0x83, 0xE0, 0x83, 0xE0, 0x83, 0xE0,                                                         // data chunk - "green", "green", "green"
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                                         // data chunk - "trans", "trans", "trans"

        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                                         // data chunk - "trans", "trans", "trans"
        0x80, 0x1F, 0x80, 0x1F, 0x80, 0x1F,                                                         // data chunk - "blue", "blue", "blue"

        0xFC, 0x00, 0x83, 0xE0, 0x80, 0x1F,                                                         // data chunk - "red", "green", "blue"
        0xFF, 0xFF, 0x00, 0x00, 0xFC, 0x00,                                                         // data chunk - "white", "trans", "red"

        0x83, 0xE0, 0x80, 0x1F, 0xFF, 0xFF,                                                         // data chunk - "green", "blue", "white"
        0x00, 0x00, 0x83, 0xE0, 0x83, 0xE0,                                                         // data chunk - "trans", "green", "green"

        0x00, 0x00,                                                                                 // end chunk - length
        0x65, 0x6E, 0x64                                                                            // end chunk - ASCII "end"
};

static uint8_t qoi_cal_data[2 * sizeof(raw_data)];
static uint8_t raw_cal_data[2 * sizeof(raw_data)];

static uint8_t received_data[sizeof(raw_data)];

static size_t qoi_encode(uint8_t *raw_buf, uint8_t *qoi_buf, size_t length);
static size_t qoi_decode(uint8_t *qoi_buf, uint8_t *raw_buf, size_t length);

#endif