cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/event_handler_task.c"
                       INCLUDE_DIRS "."
                       REQUIRES common event
                       )