#include "st7735_driver.h"
#include "data.h"

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

	if(st7735_xx_reg_init()) {
		return -1;
	}

	if(st7735_screen_write()) {
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
static int st7735_xx_reg_init(void) {
	for(int i = 0; i < sizeof(ST7735_XX_REG_ST_ARRAY) / sizeof(ST7735_XX_REG_ST_ARRAY[0]); i++) {
		if(st7735_xx_reg_write(&ST7735_XX_REG_ST_ARRAY[i])) {
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
static int st7735_xx_reg_write(struct st7735_xx_reg_st *st7735_xx_reg) {

	// write reg
	if(gpio_pin_configure_dt(&st7735_cmd_data_gpiospec, GPIO_OUTPUT_ACTIVE)){						// cmd mode
		return -1;
	}

	struct spi_buf tx_spi_buf = { .buf = st7735_xx_reg, .len = 1};
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};

	if(spi_write_dt(&st7735_spispec, &tx_spi_buf_set)) {
		return -1;
	}

	// write data
	if(st7735_xx_reg->reg_data == NULL) {
		return 0;
	} else {
		if(gpio_pin_configure_dt(&st7735_cmd_data_gpiospec, GPIO_OUTPUT_INACTIVE)){					// data mode
			return -1;
		}

		tx_spi_buf.buf = st7735_xx_reg->reg_data;
		tx_spi_buf.len = st7735_xx_reg->reg_data_size;
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
	if(st7735_screen_hours_tens_write()) {
		return -1;
	}
	if(st7735_screen_hours_units_write()) {
			return -1;
		}
	if(st7735_screen_minutes_tens_write()) {
		return -1;
	}
	if(st7735_screen_minutes_units_write()) {
		return -1;
	}
	return 0;
}

/*
 * @brief hours tens set
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int st7735_screen_hours_tens_write(void) {
	if(st7735_screen_one_position_write(6, 7, *(get_ds3231_reg_value_dec_st() + 3))) {
		return -1;
	}
	return 0;
}

/*
 * @brief hours units set
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int st7735_screen_hours_units_write(void) {
	if(st7735_screen_one_position_write(4, 5, *(get_ds3231_reg_value_dec_st() + 2))) {
		return -1;
	}
	return 0;
}

/*
 * @brief minutes tens set
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int st7735_screen_minutes_tens_write(void) {
	if(st7735_screen_one_position_write(2, 3, *(get_ds3231_reg_value_dec_st() + 1))) {
		return -1;
	}
	return 0;
}

/*
 * @brief minutes unites set
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int st7735_screen_minutes_units_write(void) {
	if(st7735_screen_one_position_write(0, 1, *get_ds3231_reg_value_dec_st())) {
		return -1;
	}
	return 0;
}

/*
 * @brief one position write
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int st7735_screen_one_position_write(int caset, int raset, int number) {
	if(st7735_xx_reg_write(&ST7735_CASET_RASET_REG_ST_ARRAY[caset])) {
		return -1;
	}
	if(st7735_xx_reg_write(&ST7735_CASET_RASET_REG_ST_ARRAY[raset])) {
		return -1;
	}
	if(st7735_xx_reg_write(&ST7735_DEFAULT_NUMBER_IMAGE_DATA_ST_ARRAY[number].st7735_ramwr_reg)) {
		return -1;
	}
	return 0;
}
