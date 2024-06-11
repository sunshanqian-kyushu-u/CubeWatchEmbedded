#include "ds3231_driver.h"
#include "data.h"

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

    // if(ds3231_time_write(17, 00)) {
	// 	return -1;
	// }

    if(ds3231_time_read()) {
        return -1;
    }
    ds3231_time_cover();

    return 0;
}

/*
 * @brief read reg(s) value
 *
 * @param reg register address
 * @param buf target buffer
 * @param length length to read
 * 
 * @retval 0 succeed
 * @retval -1 failed
 */
static int ds3231_reg_read(uint8_t reg, uint8_t *buf, uint32_t length) {
    if(i2c_write_dt(&ds3231_i2c, &reg, 1) != 0) {                                                   // write reg
        return -1;
    }
    if(i2c_read_dt(&ds3231_i2c, buf, length) != 0) {                                                // read data
        return -1;
    }
    return 0;
}

/*
 * @brief read time
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int ds3231_time_read(void) {
    if(ds3231_reg_read(DS3231_SECONDS_REG_ADDRESS, 
            get_ds3231_reg_value_bcd_st(), 7) != 0) {
        return -1;
    }
    ds3231_bcd_2_dec(get_ds3231_reg_value_bcd_st(), 
            get_ds3231_reg_value_dec_st());
    return 0;
}

void ds3231_bcd_2_dec(struct ds3231_reg_value_bcd_st *ds3231_reg_value_bcd, 
        struct ds3231_reg_value_dec_st *ds3231_reg_value_dec) {
    ds3231_reg_value_dec->minutes_units = ds3231_reg_value_bcd->minutes & 0x0F;
	ds3231_reg_value_dec->minutes_tens = ds3231_reg_value_bcd->minutes >> 4;
    if(ds3231_reg_value_bcd->hours & 0x40) {                                                        // 12h mode
        uint8_t temp_hours_bcd = ds3231_reg_value_bcd->hours & 0x1F;
        uint8_t temp_hours_dec = 10 * (temp_hours_bcd >> 4) + 
                (temp_hours_bcd & 0x0F);
        if(ds3231_reg_value_bcd->hours & 0x20) {                                                    // PM
            ds3231_reg_value_dec->hours_units = (temp_hours_dec + 12) % 10;
            ds3231_reg_value_dec->hours_tens = (temp_hours_dec + 12) / 10;
        } else {                                                                                    // AM
            ds3231_reg_value_dec->hours_units = temp_hours_dec % 10;
            ds3231_reg_value_dec->hours_tens = temp_hours_dec / 10;
        }
    } else {                                                                                        // 24h mode
        ds3231_reg_value_dec->hours_units = ds3231_reg_value_bcd->hours & 0x0F;
        ds3231_reg_value_dec->hours_tens = ds3231_reg_value_bcd->hours >> 4;
    }
}

/*
 * @brief write reg(s) value
 *
 * @param reg register address
 * @param buf data buffer will be written into register(s)
 * @param length length to write
 * 
 * @retval 0 succeed
 * @retval -1 failed
 */
static int ds3231_reg_write(uint8_t reg, uint8_t *buf, uint32_t length) {
    if(i2c_write_dt(&ds3231_i2c, &reg, 1) != 0) {                                                   // write reg
        return -1;
    }
    if(i2c_write_dt(&ds3231_i2c, buf, length) != 0) {                                               // write data
        return -1;
    }
    return 0;
}

/*
 * @brief write time
 *
 * @param buf data buffer will be written into register(s)
 * 
 * @retval 0 succeed
 * @retval -1 failed
 */
int ds3231_time_write(uint8_t hours, uint8_t minutes) {
    uint8_t temp_buf[3] = {DS3231_MINUTES_REG_ADDRESS};                                             // [0] should be address
    temp_buf[1] = minutes / 10 << 4 | minutes % 10;
    temp_buf[2] = hours / 10 << 4 | hours % 10;
    if(ds3231_reg_write(DS3231_SECONDS_REG_ADDRESS, temp_buf, sizeof(temp_buf)) != 0) {
        return -1;
    }
    return 0;
}

/*
 * @brief cover latest time to previous time
 */
void ds3231_time_cover(void) {
    memcpy(get_ds3231_reg_value_dec_previous_st(), get_ds3231_reg_value_dec_st(), 
            sizeof(struct ds3231_reg_value_dec_st));
}

/*
 * @brief if time changed
 *
 * @retval 0 not changed
 * @retval others changed
 */
int ds3231_is_time_changed(void) {
    return memcmp(get_ds3231_reg_value_dec_previous_st(), get_ds3231_reg_value_dec_st(), 
            sizeof(struct ds3231_reg_value_dec_st));
}
