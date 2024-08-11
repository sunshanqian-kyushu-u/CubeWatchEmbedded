#include "m24m02_driver.h"
#include "common.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(m24m02, LOG_LEVEL_ERR);

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
 */
int m24m02x_write(uint8_t sector, uint8_t addr_high, uint8_t addr_low, 
        uint8_t *buf, size_t length) {

    LOG_DBG("[sector %d] remains %d byte, and need %d byte...", 
            sector, (256 - addr_high - 1) * 256 + (256 - addr_low), length);

    // check if there is enough space
    if (((256 - addr_high - 1) * 256 + (256 - addr_low)) >= length) {                               // enough

        LOG_DBG("enough space...");

        uint8_t curr_addr_high = addr_high;
        uint8_t curr_addr_low = addr_low;
        size_t shift = 0;

        if (length <= (256 - curr_addr_low)) {                                                      // 1 page

            LOG_DBG("need 1 page...");
            LOG_DBG("start to write page 1...");

            memcpy(m24m02_tx_buf, buf, length);
            if (m24m02_send(sector, curr_addr_high, curr_addr_low, length)) {
                return -1;
            }

            LOG_DBG("write from 0x%.02x 0x%.02x to 0x%.02x 0x%.02x, %d byte...", 
                    curr_addr_high, curr_addr_low, curr_addr_high, 
                    curr_addr_low + length - 1, length);
            LOG_DBG("page 1 done...");
            LOG_DBG("remain 0 byte...");
            LOG_DBG("write secceed.");
            printk("\r\n");

            return 0;

        } else {                                                                                    // > 1 page

            LOG_DBG("need more than 1 page.");
            LOG_DBG("start to write page 1...");

            memcpy(m24m02_tx_buf, buf, 256 - curr_addr_low);
            if (m24m02_send(sector, curr_addr_high, curr_addr_low, 256 - curr_addr_low)) {
                return -1;
            }

            shift = 256 - curr_addr_low;

            LOG_DBG("write from 0x%.02x 0x%.02x to 0x%.02x 0xFF, %d byte...", 
                    curr_addr_high, curr_addr_low, curr_addr_high, shift);
            LOG_DBG("page 1 done...");
            LOG_DBG("remain %d byte...", length - shift);
            LOG_DBG("buf pointer moves from %p to %p...", (void *)buf, (void *)(buf + shift - 1));
            LOG_DBG("shift is %d...", shift);
            printk("\r\n");

            curr_addr_high += 1;                                                                    // will not bound
            curr_addr_low = 0x00;

            uint8_t i;

            for (i = 0; i < ((length - (256 - addr_low)) / 256); i++) {

                LOG_DBG("start to write page %d...", i + 2);

                memcpy(m24m02_tx_buf, buf + shift, 256);
                if (m24m02_send(sector, curr_addr_high, curr_addr_low, 256)) {
                    return -1;
                }

                shift += 256;

                LOG_DBG("write from 0x%.02x 0x00 to 0x%.02x 0xFF, 256 byte...", 
                        curr_addr_high, curr_addr_high);
                LOG_DBG("page %d done...", i + 2);
                LOG_DBG("remain %d byte...", length - shift);

                if (length - shift == 0) {                                                          // no data left
                    LOG_DBG("write secceed.");
                    printk("\r\n");
                    return 0;
                } else {
                    curr_addr_high += 1;                                                            // will not bound

                    LOG_DBG("buf pointer moves from %p to %p...", 
                            (void *)(buf + shift - 256), (void *)(buf + shift - 1));
                    LOG_DBG("shift is %d...", shift);
                    printk("\r\n");
                }
            }

            if (i == 0) {                                                                           // not enter the loop
                i = 2;
            } else {
                i = (length - (256 - addr_low)) / 256 + 2;
            }

            LOG_DBG("start to write page %d...", i);

            memcpy(m24m02_tx_buf, buf + shift, length - shift);
            if (m24m02_send(sector, curr_addr_high, curr_addr_low, length - shift)) {
                return -1;
            }

            LOG_DBG("write from 0x%.02x 0x00 to 0x%.02x 0x%.02x, %d byte...", 
                    curr_addr_high, curr_addr_high, length - shift - 1, length - shift);
            LOG_DBG("page %d done...", i);
            LOG_DBG("remain 0 byte...");
            LOG_DBG("write secceed.");
            printk("\r\n");

            return 0;
        }
    } else {                                                                                        // not enough

        LOG_ERR("no enough space!");
        LOG_ERR("write failed!");
        printk("\r\n");
        return -1;

    }
}

/*
 * @brief m24m02 identification page write byte
 *
 * @param addr A7-A0 address
 * @param buf data buffer
 * @param length data length
 * 
 * @retval 0 succeed
 * @retval -1 failed
 */
int m24m02e_write(uint8_t addr, uint8_t *buf, size_t length) {
    LOG_DBG("[sector e] remains %d byte, and need %d byte...", 256 - addr, length);

    if (length <= 256 - addr) {                                                                     // enough space

        LOG_DBG("enough space...");
        LOG_DBG("start to write [sector e]...");

        memcpy(m24m02_tx_buf, buf, length);
        if (m24m02_send(4, 0x00, addr, length)) {
            return -1;
        }

        LOG_DBG("write from 0x%.02x to 0x%.02x, %d byte...", addr, addr + length - 1, length);
        LOG_DBG("[sector e] done...");
        LOG_DBG("write secceed.");
        printk("\r\n");

        return 0;

    } else {

        LOG_ERR("no enough space!");
        LOG_ERR("write failed!");
        printk("\r\n");
        return -1;

    }
}

static int m24m02_send(uint8_t sector, uint8_t addr_high, uint8_t addr_low, size_t length) {

    if (length + 2 > 255) {                                                                         // should send twice
        if (m24m02_send_twice(sector, addr_high, addr_low, length)) {
            return -1;
        }
    } else {                                                                                        // send once
        if (m24m02_send_once(sector, addr_high, addr_low, length)) {
            return -1;
        }
    }
    
    return 0;
}

static int m24m02_send_twice(uint8_t sector, uint8_t addr_high, uint8_t addr_low, size_t length) {
    m24m02_tx_buf_part1[0] = addr_high;
    m24m02_tx_buf_part1[1] = addr_low;
    memcpy(m24m02_tx_buf_part1 + 2, m24m02_tx_buf, 253);

    m24m02_tx_buf_part2[0] = addr_high;
    m24m02_tx_buf_part2[1] = addr_low + 253;
    memcpy(m24m02_tx_buf_part2 + 2, m24m02_tx_buf + 253, length - 253);

    switch (sector) {
        case 0:
            if(i2c_write_dt(&m24m02a_i2c, m24m02_tx_buf_part1, 255)) {
                return -1;
            }
            k_msleep(DELAY_MS);
            if(i2c_write_dt(&m24m02a_i2c, m24m02_tx_buf_part2, length - 253 + 2)) {
                return -1;
            }
            break;
        case 1:
            if(i2c_write_dt(&m24m02b_i2c, m24m02_tx_buf_part1, 255)) {
                return -1;
            }
            k_msleep(DELAY_MS);
            if(i2c_write_dt(&m24m02b_i2c, m24m02_tx_buf_part2, length - 253 + 2)) {
                return -1;
            }
            break;
        case 2:
            if(i2c_write_dt(&m24m02c_i2c, m24m02_tx_buf_part1, 255)) {
                return -1;
            }
            k_msleep(DELAY_MS);
            if(i2c_write_dt(&m24m02c_i2c, m24m02_tx_buf_part2, length - 253 + 2)) {
                return -1;
            }
            break;
        case 3:
            if(i2c_write_dt(&m24m02d_i2c, m24m02_tx_buf_part1, 255)) {
                return -1;
            }
            k_msleep(DELAY_MS);
            if(i2c_write_dt(&m24m02d_i2c, m24m02_tx_buf_part2, length - 253 + 2)) {
                return -1;
            }
            break;
        case 4:
            if(i2c_write_dt(&m24m02e_i2c, m24m02_tx_buf_part1, 255)) {
                return -1;
            }
            k_msleep(DELAY_MS);
            if(i2c_write_dt(&m24m02e_i2c, m24m02_tx_buf_part2, length - 253 + 2)) {
                return -1;
            }
            break;
        default:
            return -1;
    }

    k_msleep(DELAY_MS);

    return 0;
}

static int m24m02_send_once(uint8_t sector, uint8_t addr_high, uint8_t addr_low, size_t length) {
    m24m02_tx_buf_part1[0] = addr_high;
    m24m02_tx_buf_part1[1] = addr_low;
    memcpy(m24m02_tx_buf_part1 + 2, m24m02_tx_buf, length);

    switch (sector) {
        case 0:
            if(i2c_write_dt(&m24m02a_i2c, m24m02_tx_buf_part1, length + 2)) {
                return -1;
            }
            break;
        case 1:
            if(i2c_write_dt(&m24m02b_i2c, m24m02_tx_buf_part1, length + 2)) {
                return -1;
            }
            break;
        case 2:
            if(i2c_write_dt(&m24m02c_i2c, m24m02_tx_buf_part1, length + 2)) {
                return -1;
            }
            break;
        case 3:
            if(i2c_write_dt(&m24m02d_i2c, m24m02_tx_buf_part1, length + 2)) {
                return -1;
            }
            break;
        case 4:
            if(i2c_write_dt(&m24m02e_i2c, m24m02_tx_buf_part1, length + 2)) {
                return -1;
            }
            break;
        default:
            return -1;
    }
    
    k_msleep(DELAY_MS);

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
int m24m02x_read(uint8_t sector, uint8_t addr_high, uint8_t addr_low, 
        uint8_t *buf, size_t length) {

    if (((256 - addr_high - 1) * 256 + (256 - addr_low)) >= length) {
        LOG_DBG("read address checked...");
        
        uint8_t curr_addr_high = addr_high;
        uint8_t curr_addr_low = addr_low;
        size_t shift = 0;

        if (length <= (256 - curr_addr_low)) {
            LOG_DBG("have 1 page...");
            LOG_DBG("start to read page 1...");

            m24m02_rx_addr[0] = curr_addr_high;
            m24m02_rx_addr[1] = curr_addr_low;
            if (m24m02_receive(sector, buf, length)) {
                return -1;
            }

            LOG_DBG("read from 0x%.02x 0x%.02x to 0x%.02x 0x%.02x, %d byte...", 
                    curr_addr_high, curr_addr_low, curr_addr_high, 
                    curr_addr_low + length - 1, length);
            LOG_DBG("page 1 done...");
            LOG_DBG("remain 0 byte...");
            LOG_DBG("read secceed.");
            printk("\r\n");

            return 0;

        } else {
            LOG_DBG("have more than 1 page...");
            LOG_DBG("start to read page 1...");

            m24m02_rx_addr[0] = curr_addr_high;
            m24m02_rx_addr[1] = curr_addr_low;
            if (m24m02_receive(sector, buf, 256 - curr_addr_low)) {
                return -1;
            }

            shift = 256 - curr_addr_low;

            LOG_DBG("read from 0x%.02x 0x%.02x to 0x%.02x 0xFF, %d byte...", 
                    curr_addr_high, curr_addr_low, curr_addr_high, shift);
            LOG_DBG("page 1 done...");
            LOG_DBG("remain %d byte...", length - shift);
            LOG_DBG("buf pointer moves from %p to %p...", (void *)buf, (void *)(buf + shift - 1));
            LOG_DBG("shift is %d...", shift);
            printk("\r\n");

            curr_addr_high += 1;                                                                    // will not bound
            curr_addr_low = 0x00;

            uint8_t i;

            for (i = 0; i < ((length - (256 - addr_low)) / 256); i++) {

                LOG_DBG("start to read page %d...", i + 2);

                m24m02_rx_addr[0] = curr_addr_high;
                m24m02_rx_addr[1] = curr_addr_low;
                if (m24m02_receive(sector, buf + shift, 256)) {
                    return -1;
                }

                shift += 256;

                LOG_DBG("read from 0x%.02x 0x00 to 0x%.02x 0xFF, 256 byte...", 
                        curr_addr_high, curr_addr_high);
                LOG_DBG("page %d done...", i + 2);
                LOG_DBG("remain %d byte...", length - shift);

                if (length - shift == 0) {                                                          // no data left
                    LOG_DBG("read secceed.");
                    printk("\r\n");
                    return 0;
                } else {
                    curr_addr_high += 1;                                                            // will not bound

                    LOG_DBG("buf pointer moves from %p to %p...", 
                            (void *)(buf + shift - 256), (void *)(buf + shift - 1));
                    LOG_DBG("shift is %d...", shift);
                    printk("\r\n");
                }
            }

            if (i == 0) {                                                                           // not enter the loop
                i = 2;
            } else {
                i = (length - (256 - addr_low)) / 256 + 2;
            }

            LOG_DBG("start to read page %d...", i);

            m24m02_rx_addr[0] = curr_addr_high;
            m24m02_rx_addr[1] = curr_addr_low;
            if (m24m02_receive(sector, buf + shift, length - shift)) {
                return -1;
            }

            LOG_DBG("read from 0x%.02x 0x00 to 0x%.02x 0x%.02x, %d byte...", 
                    curr_addr_high, curr_addr_high, length - shift - 1, length - shift);
            LOG_DBG("page %d done...", i);
            LOG_DBG("remain 0 byte...");
            LOG_DBG("read secceed.");
            printk("\r\n");

            return 0;
        }
    } else {
        LOG_ERR("read address check failed!");
        LOG_ERR("read failed!");
        printk("\r\n");
        return -1;
    }
}

/*
 * @brief m24m02 identification page read byte
 *
 * @param addr A7-A0 address
 * @param buf where data will be written to
 * @param length data length
 * 
 * @retval 0 succeed
 * @retval -1 failed
 */
int m24m02e_read(uint8_t addr, uint8_t *buf, size_t length) {
    if ((256 - addr) >= length) {
        LOG_DBG("read address checked...");
        LOG_DBG("start to read [sector e]...");

        m24m02_rx_addr[0] = 0x00;
        m24m02_rx_addr[1] = addr;
        if (m24m02_receive(4, buf, length)) {
            return -1;
        }

        LOG_DBG("read from 0x%.02x to 0x%.02x, %d byte...", addr, addr + length - 1, length);
        LOG_DBG("[sector e] done...");
        LOG_DBG("read secceed.");
        printk("\r\n");

        return 0;
    } else {

        LOG_ERR("read address check failed!");
        LOG_ERR("read failed!");
        printk("\r\n");
        return -1;

    }
}

static int m24m02_receive(uint8_t sector, uint8_t *buf, size_t length) {

    if (length > 255) {                                                                             // length = 256, should reveive twice
        if (m24m02_receive_twice(sector, buf)) {
            return -1;
        }
    } else {                                                                                        // reveive once
        if (m24m02_receive_once(sector, buf, length)) {
            return -1;
        }
    }
    
    return 0;
}

static int m24m02_receive_twice(uint8_t sector, uint8_t *buf) {

    switch (sector) {
        case 0:
            if(i2c_write_read_dt(&m24m02a_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf, 255)) {
                return -1;
            }
            k_msleep(DELAY_MS);
            m24m02_rx_addr[1] = 0xFF;
            if(i2c_write_read_dt(&m24m02a_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf + 255, 1)) {
                return -1;
            }
            break;
        case 1:
            if(i2c_write_read_dt(&m24m02b_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf, 255)) {
                return -1;
            }
            k_msleep(DELAY_MS);
            m24m02_rx_addr[1] = 0xFF;
            if(i2c_write_read_dt(&m24m02b_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf + 255, 1)) {
                return -1;
            }
            break;
        case 2:
            if(i2c_write_read_dt(&m24m02c_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf, 255)) {
                return -1;
            }
            k_msleep(DELAY_MS);
            m24m02_rx_addr[1] = 0xFF;
            if(i2c_write_read_dt(&m24m02c_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf + 255, 1)) {
                return -1;
            }
            break;
        case 3:
            if(i2c_write_read_dt(&m24m02d_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf, 255)) {
                return -1;
            }
            k_msleep(DELAY_MS);
            m24m02_rx_addr[1] = 0xFF;
            if(i2c_write_read_dt(&m24m02d_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf + 255, 1)) {
                return -1;
            }
            break;
        case 4:
            if(i2c_write_read_dt(&m24m02e_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf, 255)) {
                return -1;
            }
            k_msleep(DELAY_MS);
            m24m02_rx_addr[1] = 0xFF;
            if(i2c_write_read_dt(&m24m02e_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf + 255, 1)) {
                return -1;
            }
            break;
        default:
            return -1;
    }

    k_msleep(DELAY_MS);
    
    return 0;
}

static int m24m02_receive_once(uint8_t sector, uint8_t *buf, size_t length) {
    switch (sector) {
        case 0:
            if(i2c_write_read_dt(&m24m02a_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf, length)) {
                return -1;
            }
            break;
        case 1:
            if(i2c_write_read_dt(&m24m02b_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf, length)) {
                return -1;
            }
            break;
        case 2:
            if(i2c_write_read_dt(&m24m02c_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf, length)) {
                return -1;
            }
            break;
        case 3:
            if(i2c_write_read_dt(&m24m02d_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf, length)) {
                return -1;
            }
            break;
        case 4:
            if(i2c_write_read_dt(&m24m02e_i2c, m24m02_rx_addr, 
                    sizeof(m24m02_rx_addr), buf, length)) {
                return -1;
            }
            break;
        default:
            return -1;
    }

    k_msleep(DELAY_MS);
    
    return 0;
}
