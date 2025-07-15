cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/relay_driver.c"
                       INCLUDE_DIRS "."
                       REQUIRES common esp_driver_gpio
                       )