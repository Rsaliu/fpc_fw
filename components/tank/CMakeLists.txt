cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/tank.c"
                       INCLUDE_DIRS "."
                       REQUIRES common
                       )