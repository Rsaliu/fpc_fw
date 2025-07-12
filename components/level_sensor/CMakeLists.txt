cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/level_sensor.c"
                       INCLUDE_DIRS "."
                       REQUIRES common esp_driver_uart 
                       common esp_driver_gpio
                       )