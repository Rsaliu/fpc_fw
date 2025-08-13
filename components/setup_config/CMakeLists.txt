cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/setup_config.c" 
                       INCLUDE_DIRS "."
                       REQUIRES json spiffs tank pump level_sensor pump_monitor tank_monitor 
                                 relay_driver current_sensor common
                       )