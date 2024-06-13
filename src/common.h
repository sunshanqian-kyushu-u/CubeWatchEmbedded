#ifndef _COMMON_H_
#define _COMMON_H_

#include <zephyr/kernel.h>

int ds3231_init(void);
int ds3231_time_write(uint8_t hours, uint8_t minutes);
int ds3231_time_read(void);
void ds3231_time_cover(void);
int ds3231_is_time_changed(void);
uint8_t get_ds3231_reg_value_dec_minutes_units(void);
uint8_t get_ds3231_reg_value_dec_minutes_tens(void);
uint8_t get_ds3231_reg_value_dec_hours_units(void);
uint8_t get_ds3231_reg_value_dec_hours_tens(void);
void ds3231_reg_value_bcd_print(void);
void ds3231_reg_value_dec_print(void);
void ds3231_reg_value_dec_previous_print(void);

int st7735_init(void);
int st7735_screen_write(void);

#endif