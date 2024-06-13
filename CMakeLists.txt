cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(25_CubeWatch)

target_sources(app PRIVATE src/main.c src/ds3231_driver.c src/st7735_driver.c)
