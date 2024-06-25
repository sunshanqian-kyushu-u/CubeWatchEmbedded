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

#define SECONDS_OFFSET                      0
#define MINUTES_OFFSET                      2
#define HOURS_OFFSET                        4
#define DAY_OFFSET                          6
#define DATE_OFFSET                         9
#define MONTH_CENTURY_OFFSET                11
#define YEAR_OFFSET                         15

struct ds3231_reg_value_bcd_st {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month_century;
    uint8_t year;
};

struct ds3231_reg_value_dec_st {
    uint8_t minutes_units;
    uint8_t minutes_tens;
    uint8_t hours_units;
    uint8_t hours_tens;
};

static struct ds3231_reg_value_bcd_st ds3231_reg_value_bcd;											// get time from ds3231
static struct ds3231_reg_value_dec_st ds3231_reg_value_dec;                                         // dec time
static struct ds3231_reg_value_dec_st ds3231_reg_value_dec_previous;                                // last second's dec time

static int ds3231_control_reg_init(void);
static int ds3231_xx_reg_write(uint8_t *buf, uint32_t length);
static uint8_t get_s_m_H_M_y(uint8_t *buf, uint8_t offset);
static uint8_t ascii_2_uint8_num(uint8_t ascii);
static uint8_t get_day(uint8_t *buf, uint8_t offset);
static int ds3231_xx_reg_read(uint8_t reg, uint8_t *dst, uint32_t length);
static void ds3231_bcd_2_dec(void);

#endif