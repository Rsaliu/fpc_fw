cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/tank_monitor.c"
                       INCLUDE_DIRS "."
                       REQUIRES common tank event
                       )