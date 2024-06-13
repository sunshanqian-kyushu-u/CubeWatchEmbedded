#include "st7735_driver.h"
#include "common.h"

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>

#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

LOG_MODULE_REGISTER(st7735, LOG_LEVEL_DBG);

struct spi_dt_spec st7735_spispec = SPI_DT_SPEC_GET(DT_NODELABEL(st7735), 
        SPI_WORD_SET(8) | SPI_TRANSFER_MSB, 0);
struct gpio_dt_spec st7735_cs_gpiospec = GPIO_DT_SPEC_GET_BY_IDX(
        DT_NODELABEL(spi1), cs_gpios, 0);															// cs-gpios -> cs_gpios
struct gpio_dt_spec st7735_cmd_data_gpiospec = GPIO_DT_SPEC_GET_BY_IDX(
        DT_NODELABEL(spi1), cs_gpios, 1);
struct gpio_dt_spec st7735_bk_gpiospec = GPIO_DT_SPEC_GET_BY_IDX(
        DT_NODELABEL(spi1), cs_gpios, 2);

/*
 * @brief st7735 main init func
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int st7735_init(void) {
	if(!spi_is_ready_dt(&st7735_spispec)) {
		return -1;
	}

	if(!gpio_is_ready_dt(&st7735_cs_gpiospec)) {
		return -1;
	}

	if(!gpio_is_ready_dt(&st7735_cmd_data_gpiospec)) {
		return -1;
	}

	if(!gpio_is_ready_dt(&st7735_bk_gpiospec)) {
		return -1;
	}

	if(gpio_pin_configure_dt(&st7735_cs_gpiospec, GPIO_OUTPUT_ACTIVE)) {
		return -1;
	}

	if(gpio_pin_configure_dt(&st7735_bk_gpiospec, GPIO_OUTPUT_ACTIVE)) {
		return -1;
	}

	if(st7735_reg_init()) {
		return -1;
	}

	return 0;
}

/*
 * @brief st7735 reg init func
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
static int st7735_reg_init(void) {
	for(int i = 0; i < sizeof(ST7735_XX_REG_ST_ARRAY) / sizeof(ST7735_XX_REG_ST_ARRAY[0]); i++) {
		if(st7735_xx_reg_write(ST7735_XX_REG_ST_ARRAY[i])) {
			return -1;
		}
	}
	return 0;
}

/*
 * @brief write register func
 *
 * @param st7735_xx_reg data struct will be written to reg
 * 
 * @retval 0 succeed
 * @retval -1 failed
 */
static int st7735_xx_reg_write(const struct st7735_xx_reg_st st7735_xx_reg) {

	// write reg
	if(gpio_pin_configure_dt(&st7735_cmd_data_gpiospec, GPIO_OUTPUT_ACTIVE)){						// cmd mode
		return -1;
	}

	struct spi_buf tx_spi_buf = { .buf = (uint8_t *)&st7735_xx_reg.reg_address, .len = 1};			// avoid warning: initialization discards ‘const’ qualifier ...
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};

	if(spi_write_dt(&st7735_spispec, &tx_spi_buf_set)) {
		return -1;
	}

	// write data
	if(st7735_xx_reg.reg_data == NULL) {
		return 0;
	} else {
		if(gpio_pin_configure_dt(&st7735_cmd_data_gpiospec, GPIO_OUTPUT_INACTIVE)){					// data mode
			return -1;
		}

		tx_spi_buf.buf = (uint8_t *)st7735_xx_reg.reg_data;
		tx_spi_buf.len = st7735_xx_reg.reg_data_size;
		tx_spi_buf_set.buffers = &tx_spi_buf;
		tx_spi_buf_set.count = 1;

		if(spi_write_dt(&st7735_spispec, &tx_spi_buf_set)) {
			return -1;
		}

		return 0;
	}
}

/*
 * @brief display func
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int st7735_screen_write(void) {
	uint8_t temp_time[] = {get_ds3231_reg_value_dec_minutes_units(), 
			get_ds3231_reg_value_dec_minutes_tens(), 
			get_ds3231_reg_value_dec_hours_units(), 
			get_ds3231_reg_value_dec_hours_tens()};
	for(int i = 0; i < 4; i++) {
		if(st7735_screen_one_position_write(i, temp_time[i])) {
			return -1;
		}
	}
	return 0;
}

/*
 * @brief one position write
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
static int st7735_screen_one_position_write(int index, int number) {
	if(st7735_xx_reg_write(ST7735_CASET_RASET_REG_ST_ARRAY[2 * index])) {
		return -1;
	}
	if(st7735_xx_reg_write(ST7735_CASET_RASET_REG_ST_ARRAY[2 * index + 1])) {
		return -1;
	}
	if(st7735_xx_reg_write(ST7735_DEFAULT_NUMBER_IMAGE_DATA_ST_ARRAY[number].st7735_ramwr_reg)) {
		return -1;
	}
	return 0;
}
