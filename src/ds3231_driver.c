#include "ds3231_driver.h"
#include "common.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ds3231, LOG_LEVEL_DBG);

static const struct i2c_dt_spec ds3231_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(ds3231));

/*
 * @brief ds3231 init func
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int ds3231_init(void) {
    if(!device_is_ready(ds3231_i2c.bus)) {
        return -1;
    }

    if(ds3231_control_reg_init()) {
        return -1;
    }

    if(ds3231_time_read()) {
        return -1;
    }
    ds3231_time_cover();

    return 0;
}

/*
 * @brief control register init func
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
static int ds3231_control_reg_init(void) {
    ds3231_tx_buf[0] = DS3231_CONTROL_REG_ADDRESS;
    ds3231_tx_buf[1] = 0x1C;

    return ds3231_write(2);
}

/*
 * @brief write reg(s) value
 *
 * @param length length to write
 * 
 * @retval 0 succeed
 * @retval -1 failed
 */
static int ds3231_write(size_t length) {
    return i2c_write_dt(&ds3231_i2c, ds3231_tx_buf, length);
}

/*
 * @brief write time
 *
 * @param buf data buffer will be written into register(s)
 * 
 * @retval 0 succeed
 * @retval -1 failed
 */
int ds3231_time_write(uint8_t seconds, uint8_t minutes, uint8_t hours, 
        uint8_t day, uint8_t date, uint8_t month_century, uint8_t year) {
    ds3231_tx_buf[0] = DS3231_SECONDS_REG_ADDRESS;
    ds3231_tx_buf[1] = seconds;                                                                     // SECONDS
    ds3231_tx_buf[2] = minutes;                                                                     // MINUTES
    ds3231_tx_buf[3] = hours;                                                                       // HOURS
    ds3231_tx_buf[4] = day;                                                                         // DAY
    ds3231_tx_buf[5] = date;                                                                        // DATE
    ds3231_tx_buf[6] = month_century;                                                               // MONTH_CENTURY
    ds3231_tx_buf[7] = year;                                                                        // YEAR

    return ds3231_write(8);
}

/*
 * @brief read from reg(s)
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int ds3231_time_read(void) {
    if (i2c_write_read_dt(&ds3231_i2c, ds3231_rx_addr, 1, &ds3231_bcd_time_curr, 7)) {
        return -1;
    }
    ds3231_time_bcd_2_dec();
    return 0;
}

/*
 * @brief cover latest time to previous time
 */
void ds3231_time_cover(void) {
    memcpy(&ds3231_bcd_time_prev, &ds3231_bcd_time_curr, sizeof(struct ds3231_time_st));
}

/*
 * @brief bcd data to dec data
 */
static void ds3231_time_bcd_2_dec(void) {
    ds3231_dec_time_curr.seconds = ds3231_bcd_2_dec(ds3231_bcd_time_curr.seconds);
    ds3231_dec_time_curr.minutes = ds3231_bcd_2_dec(ds3231_bcd_time_curr.minutes);
    ds3231_dec_time_curr.hours = ds3231_bcd_2_dec(ds3231_bcd_time_curr.hours);
    ds3231_dec_time_curr.day = ds3231_bcd_2_dec(ds3231_bcd_time_curr.day);
    ds3231_dec_time_curr.date = ds3231_bcd_2_dec(ds3231_bcd_time_curr.date);
    ds3231_dec_time_curr.month_century = ds3231_bcd_2_dec(ds3231_bcd_time_curr.month_century);
    ds3231_dec_time_curr.year = ds3231_bcd_2_dec(ds3231_bcd_time_curr.year);
}

/*
 * @brief bcd data to dec data
 */
static uint8_t ds3231_bcd_2_dec(uint8_t bcd) {

    uint8_t temp_tens = bcd >> 4;
    uint8_t temp_units = bcd & 0x0F;

    return temp_tens * 10 + temp_units;
}

/*
 * @brief if time changed
 *
 * @retval 0 not changed
 * @retval others changed
 */
int ds3231_is_time_changed(void) {
    return memcmp(&ds3231_bcd_time_curr + DS3231_MINUTES_REG_ADDRESS, 
            &ds3231_bcd_time_prev + DS3231_MINUTES_REG_ADDRESS, DS3231_YEAR_REG_ADDRESS);
}

/*
 * @brief get minutes units
 *
 * @return uint8_t minutes units
 */
uint8_t ds3231_get_time_minutes_units(void) {
    return ds3231_dec_time_curr.minutes % 10;
}

/*
 * @brief get minutes tens
 *
 * @return uint8_t minutes tens
 */
uint8_t ds3231_get_time_minutes_tens(void) {
    return ds3231_dec_time_curr.minutes / 10;
}

/*
 * @brief get hours units
 *
 * @return uint8_t hours units
 */
uint8_t ds3231_get_time_hours_units(void) {
    return ds3231_dec_time_curr.hours % 10;
}

/*
 * @brief get hours tens
 *
 * @return uint8_t hours tens
 */
uint8_t ds3231_get_time_hours_tens(void) {
    return ds3231_dec_time_curr.hours / 10;
}

/*
 * @brief print bcd current time
 */
void ds3231_bcd_time_curr_print(void) {
    LOG_DBG("bcd time current [seconds] is: 0x%.02x", ds3231_bcd_time_curr.seconds);
	LOG_DBG("bcd time current [minutes] is: 0x%.02x", ds3231_bcd_time_curr.minutes);
    LOG_DBG("bcd time current [hours] is: 0x%.02x", ds3231_bcd_time_curr.hours);
    LOG_DBG("bcd time current [day] is: 0x%.02x", ds3231_bcd_time_curr.day);
    LOG_DBG("bcd time current [date] is: 0x%.02x", ds3231_bcd_time_curr.date);
    LOG_DBG("bcd time current [month_century] is: 0x%.02x", ds3231_bcd_time_curr.month_century);
    LOG_DBG("bcd time current [year] is: 0x%.02x", ds3231_bcd_time_curr.year);
}

/*
 * @brief print dec current time
 */
void ds3231_dec_time_curr_print(void) {
    LOG_DBG("dec time current [seconds] is: %d", ds3231_dec_time_curr.seconds);
	LOG_DBG("dec time current [minutes] is: %d", ds3231_dec_time_curr.minutes);
    LOG_DBG("dec time current [hours] is: %d", ds3231_dec_time_curr.hours);
    LOG_DBG("dec time current [day] is: %d", ds3231_dec_time_curr.day);
    LOG_DBG("dec time current [date] is: %d", ds3231_dec_time_curr.date);
    LOG_DBG("dec time current [month_century] is: %d", ds3231_dec_time_curr.month_century);
    LOG_DBG("dec time current [year] is: %d", ds3231_dec_time_curr.year);
}
