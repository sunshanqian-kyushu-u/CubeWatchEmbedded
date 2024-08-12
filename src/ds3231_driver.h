#ifndef _DS3231_DRIVER_H_
#define _DS3231_DRIVER_H_

#include <zephyr/kernel.h>

#define DS3231_SECONDS_REG_ADDRESS          0x00
#define DS3231_MINUTES_REG_ADDRESS          0x01
#define DS3231_HOURS_REG_ADDRESS            0x02
#define DS3231_DAY_REG_ADDRESS              0x03
#define DS3231_DATE_REG_ADDRESS             0x04
#define DS3231_MONTH_CENTURY_REG_ADDRESS    0x05
#define DS3231_YEAR_REG_ADDRESS             0x06
#define DS3231_CONTROL_REG_ADDRESS			0x0E

#define DS3231_TX_BUF_SIZE_MAX 20                                                                   // 19 register + addr
#define DS3231_RX_ADDR_BUF_SIZE 1

static uint8_t ds3231_tx_buf[DS3231_TX_BUF_SIZE_MAX];
static uint8_t ds3231_rx_addr[DS3231_RX_ADDR_BUF_SIZE];

struct ds3231_time_st {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month_century;
    uint8_t year;
};

static struct ds3231_time_st ds3231_bcd_time_curr;											        // current bcd time
static struct ds3231_time_st ds3231_bcd_time_prev;											        // previous bcd time
static struct ds3231_time_st ds3231_dec_time_curr;                                                  // current dec time

static int ds3231_control_reg_init(void);
static int ds3231_write(size_t length);
static void ds3231_time_bcd_2_dec(void);
static uint8_t ds3231_bcd_2_dec(uint8_t bcd);

#endif