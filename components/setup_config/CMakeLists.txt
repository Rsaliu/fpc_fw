cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/setup_config.c" 
                       INCLUDE_DIRS "."
                       REQUIRES json spiffs tank pump tank_monitor pump_monitor current_sensor
                                relay_driver level_sensor rs485 protocol
                       )