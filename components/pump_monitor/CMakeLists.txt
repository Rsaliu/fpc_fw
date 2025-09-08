cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/pump_monitor.c"
                       INCLUDE_DIRS "."
                       REQUIRES common pump current_sensor event
                       )
