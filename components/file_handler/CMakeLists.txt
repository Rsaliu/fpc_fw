cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/file_handler.c"
                       INCLUDE_DIRS "."
                       REQUIRES common json spiffs
                       )

