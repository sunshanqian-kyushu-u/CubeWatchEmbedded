#include "led.h"
#include "common.h"

#include <zephyr/kernel.h>

#include <zephyr/drivers/gpio.h>

struct gpio_dt_spec led_gpiospec = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);

/*
 * @brief led init func
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int led_init(void) {

	if(!gpio_is_ready_dt(&led_gpiospec)) {
		return -1;
	}

    if(gpio_pin_configure_dt(&led_gpiospec, GPIO_OUTPUT_INACTIVE)) {
		return -1;
	}

	return 0;
}

int led_on(void) {
    if(gpio_pin_configure_dt(&led_gpiospec, GPIO_OUTPUT_ACTIVE)) {
		return -1;
	}

    return 0;
}

int led_off(void) {
    if(gpio_pin_configure_dt(&led_gpiospec, GPIO_OUTPUT_INACTIVE)) {
		return -1;
	}

    return 0;
}
