#ifndef _DS3231_DRIVER_H_
#define _DS3231_DRIVER_H_

#include "data.h"
#include <zephyr/kernel.h>

#define DS3231_SECONDS_REG_ADDRESS          0x00
#define DS3231_MINUTES_REG_ADDRESS          0x01
#define DS3231_HOURS_REG_ADDRESS            0x02
#define DS3231_DAY_REG_ADDRESS              0x03
#define DS3231_DATE_REG_ADDRESS             0x04
#define DS3231_MONTH_CENTURY_REG_ADDRESS    0x05
#define DS3231_YEAR_REG_ADDRESS             0x06

int ds3231_init(void);
static int ds3231_reg_read(uint8_t reg, uint8_t *dst, uint32_t length);
int ds3231_time_read(void);
void ds3231_bcd_2_dec(struct ds3231_reg_value_bcd_st *ds3231_reg_value_bcd, 
        struct ds3231_reg_value_dec_st *ds3231_reg_value_dec);
static int ds3231_reg_write(uint8_t reg, uint8_t *buf, uint32_t length);
int ds3231_time_write(uint8_t hours, uint8_t minutes);
void ds3231_time_cover(void);
int ds3231_is_time_changed(void);

#endif