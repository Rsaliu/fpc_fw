cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/factory.c" 
                            "src/globals.c"
                       INCLUDE_DIRS "."
                       REQUIRES current_sensor common setup_config pump tank level_sensor
                       )