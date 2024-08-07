#include "m24m02_driver.h"
#include "common.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(m24m02, LOG_LEVEL_DBG);

/*
 * @brief m24m02 is a 2Mbit EEPROM, 
 *        A0-A17 address bits, 2 ^ 8 * 2 ^ 10 = 256K, 
 *        each address bit corresponding 1 byte data, 
 *        256K * 8 bit = 2Mbit. 
 */
static const struct i2c_dt_spec m24m02a_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(m24m02a));
/*
 * @brief m24m02b (same as m24m02a, m24m02c, m24m02d) can be regarded as 512Kbit memory, 
 *        A0-A15 address bits, 2 ^ 6 * 2 ^ 10 = 64K, 
 *        each address bit corresponding 1 byte data, 
 *        64K * 8 bit = 512Kbit. 
 */
static const struct i2c_dt_spec m24m02b_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(m24m02b));
static const struct i2c_dt_spec m24m02c_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(m24m02c));
static const struct i2c_dt_spec m24m02d_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(m24m02d));
/*
 * @brief m24m02e is the Identification Page (256 byte), 
 *        A0-A7 address bit, 2 ^ 8 = 256, 
 *        each address bit corresponding 1 byte data, 
 *        256 * 8 bit = 256 byte. 
 */
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
 * @brief m24m02 write byte
 *
 * @param sector a = 0, b = 1, c = 2 and d = 3
 * @param addr_high A15-A8 address
 * @param addr_low A7-A0 address
 * @param buf data buffer
 * @param length data length
 * 
 * @retval 0 succeed
 * @retval -1 failed
 * 
 * @warning "roll-over" may occurs
 */
int m24m02_write(uint8_t sector, uint8_t addr_high, uint8_t addr_low, 
        uint8_t *buf, uint8_t length) {

    m24m02_tx_buf[0] = addr_high;
    m24m02_tx_buf[1] = addr_low;
    memcpy(m24m02_tx_buf + 2, buf, length);
    
    switch (sector) {
        case 0:
            if(i2c_write_dt(&m24m02a_i2c, m24m02_tx_buf, length + 2)) {
                return -1;
            }
            break;
        case 1:
            if(i2c_write_dt(&m24m02b_i2c, m24m02_tx_buf, length + 2)) {
                return -1;
            }
            break;
        case 2:
            if(i2c_write_dt(&m24m02c_i2c, m24m02_tx_buf, length + 2)) {
                return -1;
            }
            break;
        case 3:
            if(i2c_write_dt(&m24m02d_i2c, m24m02_tx_buf, length + 2)) {
                return -1;
            }
            break;
        case 4:
            if(i2c_write_dt(&m24m02e_i2c, m24m02_tx_buf, length + 2)) {
                return -1;
            }
            break;
        default:
            return -1;
    }

    k_msleep(8);

    return 0;
}

/*
 * @brief m24m02 read byte
 *
 * @param sector a, b, c and d
 * @param addr_high A15-A8 address
 * @param addr_low A7-A0 address
 * @param buf where data will be written to
 * @param length data length
 * 
 * @retval 0 succeed
 * @retval -1 failed
 */
int m24m02_read(uint8_t sector, uint8_t addr_high, uint8_t addr_low, 
        uint8_t *buf, uint8_t length) {

    uint8_t temp_buf[] = {addr_high, addr_low};
    
    switch (sector) {
        case 0:
            if(i2c_write_read_dt(&m24m02a_i2c, temp_buf, sizeof(temp_buf), buf, length)) {
                return -1;
            }
            break;
        case 1:
            if(i2c_write_read_dt(&m24m02b_i2c, temp_buf, sizeof(temp_buf), buf, length)) {
                return -1;
            }
            break;
        case 2:
            if(i2c_write_read_dt(&m24m02c_i2c, temp_buf, sizeof(temp_buf), buf, length)) {
                return -1;
            }
            break;
        case 3:
            if(i2c_write_read_dt(&m24m02d_i2c, temp_buf, sizeof(temp_buf), buf, length)) {
                return -1;
            }
            break;
        case 4:
            if(i2c_write_read_dt(&m24m02e_i2c, temp_buf, sizeof(temp_buf), buf, length)) {
                return -1;
            }
            break;
        default:
            return -1;
    }

    k_msleep(8);

    return 0;
}
