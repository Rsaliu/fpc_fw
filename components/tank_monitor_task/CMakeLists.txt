cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/tank_monitor_task.c" "src/example.c"
                       INCLUDE_DIRS "."
                       REQUIRES common tank_monitor protocol rs485
                       )