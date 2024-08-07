#ifndef _M24M02_DRIVER_H_
#define _M24M02_DRIVER_H_

#include <zephyr/kernel.h>

#define M24M02_TX_BUF_SIZE_MAX 258                                                                  // 256 (one page max) + 2 (addr_high + addr_low)
#define M24M02_RX_BUF_SIZE_MAX 256

static uint8_t m24m02_tx_buf[M24M02_TX_BUF_SIZE_MAX];
// static uint8_t m24m02_rx_buf[M24M02_RX_BUF_SIZE_MAX];

#endif