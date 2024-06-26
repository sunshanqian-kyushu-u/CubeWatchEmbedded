#include "m24m02_driver.h"
#include "common.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(m24m02, LOG_LEVEL_DBG);

static const struct i2c_dt_spec m24m02a_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(m24m02a));
static const struct i2c_dt_spec m24m02b_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(m24m02b));
static const struct i2c_dt_spec m24m02c_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(m24m02c));
static const struct i2c_dt_spec m24m02d_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(m24m02d));
static const struct i2c_dt_spec m24m02e_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(m24m02e));

/*
 * @brief m24m02 init function
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int m24m02_init(void) {
    if(!device_is_ready(m24m02a_i2c.bus)) {
        return -1;
    }

    if(!device_is_ready(m24m02b_i2c.bus)) {
        return -1;
    }

    if(!device_is_ready(m24m02c_i2c.bus)) {
        return -1;
    }

    if(!device_is_ready(m24m02d_i2c.bus)) {
        return -1;
    }

    if(!device_is_ready(m24m02e_i2c.bus)) {
        return -1;
    }

    return 0;
}

/*
 * @brief m24m02 write a byte into memory array
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int m24m02x_ma_byte_write(uint8_t sector, uint8_t addr_high, uint8_t addr_low, 
        uint8_t data) {                                                                             // ma: memory array

    uint8_t temp_buf[] = {addr_high, addr_low, data};
    
    switch (sector)
    {
    case 0:
        if(i2c_write_dt(&m24m02a_i2c, temp_buf, sizeof(temp_buf))) {
            return -1;
        }
        break;
    case 1:
        if(i2c_write_dt(&m24m02b_i2c, temp_buf, sizeof(temp_buf))) {
            return -1;
        }
        break;
    case 2:
        if(i2c_write_dt(&m24m02c_i2c, temp_buf, sizeof(temp_buf))) {
            return -1;
        }
        break;
    case 3:
        if(i2c_write_dt(&m24m02d_i2c, temp_buf, sizeof(temp_buf))) {
            return -1;
        }
        break;
    case 4:
        if(i2c_write_dt(&m24m02e_i2c, temp_buf, sizeof(temp_buf))) {
            return -1;
        }
        break;
    default:
        return -1;
    }

    k_msleep(10);

    return 0;
}

/*
 * @brief m24m02 read a byte from memory array
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int m24m02x_ma_byte_read(uint8_t sector, uint8_t addr_high, uint8_t addr_low, uint8_t *buf) {

    uint8_t temp_buf[] = {addr_high, addr_low};
    
    switch (sector)
    {
    case 0:
        if(i2c_write_read_dt(&m24m02a_i2c, temp_buf, sizeof(temp_buf), buf, 1)) {
            return -1;
        }
        break;
    case 1:
        if(i2c_write_read_dt(&m24m02b_i2c, temp_buf, sizeof(temp_buf), buf, 1)) {
            return -1;
        }
        break;
    case 2:
        if(i2c_write_read_dt(&m24m02c_i2c, temp_buf, sizeof(temp_buf), buf, 1)) {
            return -1;
        }
        break;
    case 3:
        if(i2c_write_read_dt(&m24m02d_i2c, temp_buf, sizeof(temp_buf), buf, 1)) {
            return -1;
        }
        break;
    case 4:
        if(i2c_write_read_dt(&m24m02e_i2c, temp_buf, sizeof(temp_buf), buf, 1)) {
            return -1;
        }
        break;
    default:
        return -1;
    }

    k_msleep(10);

    return 0;
}
