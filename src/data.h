#ifndef _DATA_H_
#define _DATA_H_

#include <zephyr/kernel.h>

struct ds3231_reg_value_bcd_st {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month_century;
    uint8_t year;
};

static struct ds3231_reg_value_bcd_st ds3231_reg_value_bcd;                                         // get time from ds3231

struct ds3231_reg_value_dec_st {
    uint8_t minutes_units;
    uint8_t minutes_tens;
    uint8_t hours_units;
    uint8_t hours_tens;
};

static struct ds3231_reg_value_dec_st ds3231_reg_value_dec;                                         // dec time
static struct ds3231_reg_value_dec_st ds3231_reg_value_dec_previous;                                // last second's dec time

uint8_t* get_ds3231_reg_value_bcd_st(void);
uint8_t* get_ds3231_reg_value_dec_st(void);
uint8_t* get_ds3231_reg_value_dec_previous_st(void);
void ds3231_reg_value_bcd_print(void);
void ds3231_reg_value_dec_print(void);
void ds3231_reg_value_dec_previous_print(void);
uint8_t* get_ds3231_set_time(void);

#endif