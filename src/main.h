#ifndef _MAIN_H_
#define _MAIN_H_

#include <zephyr/kernel.h>

#define STACKSIZE 1024

#define WRITE_SCREEN_PRIORITY 7

static void write_screen_thread(void);

#endif
