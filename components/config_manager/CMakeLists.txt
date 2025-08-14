cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/validate_json.c"
                       INCLUDE_DIRS "."
                       REQUIRES common json spiffs
                       )


