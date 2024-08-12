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
	if(st7735_write(ST7735_SLPOUT_REG_BUF, sizeof(ST7735_SLPOUT_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_FRMCTR1_REG_BUF, sizeof(ST7735_FRMCTR1_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_FRMCTR2_REG_BUF, sizeof(ST7735_FRMCTR2_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_FRMCTR3_REG_BUF, sizeof(ST7735_FRMCTR3_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_INVCTR_REG_BUF, sizeof(ST7735_INVCTR_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_PWCTR1_REG_BUF, sizeof(ST7735_PWCTR1_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_PWCTR2_REG_BUF, sizeof(ST7735_PWCTR2_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_PWCTR3_REG_BUF, sizeof(ST7735_PWCTR3_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_PWCTR4_REG_BUF, sizeof(ST7735_PWCTR4_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_PWCTR5_REG_BUF, sizeof(ST7735_PWCTR5_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_MADCTL_REG_BUF, sizeof(ST7735_MADCTL_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_GMCTRP1_REG_BUF, sizeof(ST7735_GMCTRP1_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_GMCTRN1_REG_BUF, sizeof(ST7735_GMCTRN1_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_CASET_REG_BUF, sizeof(ST7735_CASET_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_RASET_REG_BUF, sizeof(ST7735_RASET_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_COLMOD_REG_BUF, sizeof(ST7735_COLMOD_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_DISPON_REG_BUF, sizeof(ST7735_DISPON_REG_BUF))) {
		return -1;
	}

	if(st7735_write(ST7735_WHITE_CANVAS_BUF, sizeof(ST7735_WHITE_CANVAS_BUF))) {
		return -1;
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
static int st7735_write(uint8_t *buf, size_t length) {

	// write reg
	if(gpio_pin_configure_dt(&st7735_cmd_data_gpiospec, GPIO_OUTPUT_ACTIVE)){						// cmd mode
		return -1;
	}

	struct spi_buf tx_spi_buf = { .buf = buf, .len = 1};
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};

	if(spi_write_dt(&st7735_spispec, &tx_spi_buf_set)) {
		return -1;
	}

	if(length == 1) {																				// no extra data

		return 0;

	} else {
		// write data
		if(gpio_pin_configure_dt(&st7735_cmd_data_gpiospec, GPIO_OUTPUT_INACTIVE)){					// data mode
			return -1;
		}

		tx_spi_buf.buf = buf + 1;
		tx_spi_buf.len = length - 1;
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
// int st7735_screen_write(void) {
// 	uint8_t temp_time[] = {ds3231_get_time_minutes_units(), 
// 			ds3231_get_time_minutes_tens(), 
// 			ds3231_get_time_hours_units(), 
// 			ds3231_get_time_hours_tens()};
// 	for(int i = 0; i < 4; i++) {
// 		if(st7735_screen_one_position_write(i, temp_time[i])) {
// 			return -1;
// 		}
// 	}
// 	return 0;
// }

/*
 * @brief one position write
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
// static int st7735_screen_one_position_write(int index, int number) {
// 	if(st7735_xx_reg_write(ST7735_CASET_RASET_REG_ST_ARRAY[2 * index])) {
// 		return -1;
// 	}
// 	if(st7735_xx_reg_write(ST7735_CASET_RASET_REG_ST_ARRAY[2 * index + 1])) {
// 		return -1;
// 	}
// 	if(st7735_xx_reg_write(ST7735_DEFAULT_NUMBER_IMAGE_DATA_ST_ARRAY[number].st7735_ramwr_reg)) {
// 		return -1;
// 	}
// 	return 0;
// }
