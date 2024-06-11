#include "main.h"
#include "ds3231_driver.h"

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

void write_screen_thread(void) {
	while(1) {
		// read time
		ds3231_time_read();

		if(ds3231_is_time_changed()) {
			ds3231_time_cover();
			// display
			st7735_screen_write();
		}

		k_msleep(1000);
	}
}

K_THREAD_DEFINE(write_screen_thread_id, STACKSIZE, write_screen_thread, NULL, NULL, NULL, WRITE_SCREEN_PRIORITY, 0, 3000);

int main(void)
{
	if(ds3231_init() != 0) {
		LOG_ERR("ds3231 init failed!");
		return -1;
	}
	LOG_DBG("ds3231 init succeed!");

	if(st7735_init() != 0) {
		LOG_ERR("st7735 init failed!");
		return -1;
	}
	LOG_DBG("st7735 init succeed!");

    return 0;
}
