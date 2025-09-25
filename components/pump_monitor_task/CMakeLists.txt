cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/pump_monitor_task.c"
                       INCLUDE_DIRS "."
                       REQUIRES common pump_monitor event event_handler_task
                       )

