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
 * red - b1111110000000000 = 0xFC00
 * green - b1000001111100000 = 0x83E0
 * blue - b1000000000011111 = 0x801F
 * trans = transparent - b0000000000000000 = 0x0000
 */
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

uint8_t raw_data[] = {
        0x72, 0x61, 0x77,                               // ASCII "raw"

        0x00, 0x03,                                     // header chunk - length
        0x68, 0x64, 0x72,                               // header chunk - ASCII "hdr"
        0x04,                                           // header chunk - width
        0x04,                                           // header chunk - height
        0x08,                                           // header chunk - channel

        0x00, 0x20,                                     // data chunk - length
        0x64, 0x61, 0x74,                               // data chunk - ASCII "dat"
        0xFC, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, // data chunk - "red", "red",
                                                        //              "trans", "trans"
        0x00, 0x00, 0x00, 0x00, 0x83, 0xE0, 0x83, 0xE0, // data chunk - "trans", "trans", 
                                                        //              "green", "green"
        0x00, 0x00, 0x00, 0x00, 0x80, 0x1F, 0x80, 0x1F, // data chunk - "trans", "trans", 
                                                        //              "blue", "blue"
        0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // data chunk - "white", "white", 
                                                        //              "trans", "trans"

        0x00, 0x00,                                     // end chunk - length
        0x65, 0x6E, 0x64                                // end chunk - ASCII "end"
};

// uint8_t qoi_data[] = {
//         0x71, 0x6F, 0x69,               // ASCII "qoi"
//         0x00, 0x03,                     // header chunk - length
//         0x68, 0x64, 0x72,               // header chunk - ASCII "hdr"
//         0x04,                           // header chunk - width
//         0x04,                           // header chunk - height
//         0x09,                           // header chunk - channel

// };

// static int qoi_encode(struct image_st *raw_image, uint8_t *des_buff);

#endif