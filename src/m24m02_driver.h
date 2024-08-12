#ifndef _M24M02_DRIVER_H_
#define _M24M02_DRIVER_H_

#include <zephyr/kernel.h>

#define DEBUG_MODE 0                                                                                // 1: true
#if DEBUG_MODE

#define DELAY_MS 100

#else

#define DELAY_MS 8

#endif



#define M24M02_TX_BUF_SIZE_MAX 256
#define M24M02_PART_BUF_SIZE_MAX 255                                                                // 256 - 1 byte Dev sel
#define M24M02_RX_ADDR_BUF_SIZE 2

static uint8_t m24m02_tx_buf[M24M02_TX_BUF_SIZE_MAX];
static uint8_t m24m02_tx_buf_part1[M24M02_PART_BUF_SIZE_MAX];
static uint8_t m24m02_tx_buf_part2[M24M02_PART_BUF_SIZE_MAX];
static uint8_t m24m02_rx_addr[M24M02_RX_ADDR_BUF_SIZE];

static int m24m02_send(uint8_t sector, uint8_t addr_high, uint8_t addr_low, size_t length);
static int m24m02_send_twice(uint8_t sector, uint8_t addr_high, uint8_t addr_low, size_t length);
static int m24m02_send_once(uint8_t sector, uint8_t addr_high, uint8_t addr_low, size_t length);
static int m24m02_receive(uint8_t sector, uint8_t *buf, size_t length);
static int m24m02_receive_twice(uint8_t sector, uint8_t *buf);
static int m24m02_receive_once(uint8_t sector, uint8_t *buf, size_t length);

#endif