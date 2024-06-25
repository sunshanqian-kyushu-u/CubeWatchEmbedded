#include "main.h"
#include "common.h"

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static void write_screen_thread(void) {
	while(1) {
		// read time
		ds3231_time_read();

		if(ds3231_is_time_changed()) {
			ds3231_time_cover();
			// display
			st7735_screen_write();
		}
		LOG_DBG("running...");
		k_msleep(1000);
	}
}

K_THREAD_DEFINE(write_screen_thread_id, STACKSIZE, write_screen_thread, 
		NULL, NULL, NULL, WRITE_SCREEN_PRIORITY, 0, 0);

void write_screen_thread_suspend(void) {
	k_thread_suspend(write_screen_thread_id);
}

void write_screen_thread_resume(void) {
	k_thread_resume(write_screen_thread_id);
}

int main(void)
{
	if(ds3231_init() != 0) {
		LOG_ERR("ds3231 init failed!");
		return -1;
	}

	uint8_t date_str[17] = "452416Tue25062024";
	if(ds3231_time_write(date_str)) {
		LOG_ERR("time write failed!");
		return -1;
	}

	LOG_DBG("ds3231 init succeed!");

	if(st7735_init() != 0) {
		LOG_ERR("st7735 init failed!");
		return -1;
	}
	LOG_DBG("st7735 init succeed!");

	if(nrf52832_init() != 0) {
		LOG_ERR("nrf52832 init failed!");
		return -1;
	}

	ds3231_time_read();
	st7735_screen_write();

    return 0;
}
