#include "data.h"

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(data, LOG_LEVEL_DBG);

uint8_t* get_ds3231_reg_value_bcd_st(void) {
    return &ds3231_reg_value_bcd;
}

uint8_t* get_ds3231_reg_value_dec_st(void) {
    return &ds3231_reg_value_dec;
}

uint8_t* get_ds3231_reg_value_dec_previous_st(void) {
    return &ds3231_reg_value_dec_previous;
}

void ds3231_reg_value_bcd_print(void) {
    LOG_DBG("seconds reg is: 0x%.02x", *get_ds3231_reg_value_bcd_st());
    LOG_DBG("minutes reg is: 0x%.02x", *(get_ds3231_reg_value_bcd_st() + 1));
	LOG_DBG("hours reg is: 0x%.02x", *(get_ds3231_reg_value_bcd_st() + 2));
	LOG_DBG("");
}

void ds3231_reg_value_dec_print(void) {
    LOG_DBG("minutes units: %d", *get_ds3231_reg_value_dec_st());
	LOG_DBG("minutes tens: %d", *(get_ds3231_reg_value_dec_st() + 1));
	LOG_DBG("hours units: %d", *(get_ds3231_reg_value_dec_st() + 2));
	LOG_DBG("hours tens: %d", *(get_ds3231_reg_value_dec_st() + 3));
	LOG_DBG("");
}

void ds3231_reg_value_dec_previous_print(void) {
    LOG_DBG("minutes units: %d", *get_ds3231_reg_value_dec_previous_st());
	LOG_DBG("minutes tens: %d", *(get_ds3231_reg_value_dec_previous_st() + 1));
	LOG_DBG("hours units: %d", *(get_ds3231_reg_value_dec_previous_st() + 2));
	LOG_DBG("hours tens: %d", *(get_ds3231_reg_value_dec_previous_st() + 3));
	LOG_DBG("");
}
