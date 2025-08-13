cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/pump_control_unit.c"
                       INCLUDE_DIRS "."
                       REQUIRES common tank_monitor pump rs485 protocol event emhashmap_wrapper relay_driver current_sensor
                       )