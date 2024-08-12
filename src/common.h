#ifndef _COMMON_H_
#define _COMMON_H_

#include <zephyr/kernel.h>

int ds3231_init(void);
int ds3231_time_write(uint8_t seconds, uint8_t minutes, uint8_t hours, 
        uint8_t day, uint8_t date, uint8_t month_century, uint8_t year);
int ds3231_time_read(void);
void ds3231_time_cover(void);
int ds3231_is_time_changed(void);
uint8_t ds3231_get_time_minutes_units(void);
uint8_t ds3231_get_time_minutes_tens(void);
uint8_t ds3231_get_time_hours_units(void);
uint8_t ds3231_get_time_hours_tens(void);
void ds3231_bcd_time_curr_print(void);
void ds3231_dec_time_curr_print(void);

int st7735_init(void);
// int st7735_screen_write(void);

int nrf52832_init(void);

void write_screen_thread_suspend(void);
void write_screen_thread_resume(void);

int m24m02_init(void);
int m24m02x_write(uint8_t sector, uint8_t addr_high, uint8_t addr_low, uint8_t *buf, size_t length);
int m24m02e_write(uint8_t addr, uint8_t *buf, size_t length);
int m24m02x_read(uint8_t sector, uint8_t addr_high, uint8_t addr_low, uint8_t *buf, size_t length);
int m24m02e_read(uint8_t addr, uint8_t *buf, size_t length);

void qoi_init(void);

int led_init(void);
int led_on(void);
int led_off(void);

#endif