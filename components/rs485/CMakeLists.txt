cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/rs485.c" "src/rs485_context.c" 
                       INCLUDE_DIRS "."
                       REQUIRES common esp_driver_uart 
                       common esp_driver_gpio
                       )