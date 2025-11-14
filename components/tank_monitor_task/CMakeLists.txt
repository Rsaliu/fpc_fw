cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/tank_monitor_task.c" 
                       INCLUDE_DIRS "."
                       REQUIRES common tank_monitor event_handler_task pump_control_unit relay_driver
                       )