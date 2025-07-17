cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/protocol.c" 
                       INCLUDE_DIRS "."
                       REQUIRES common crc_calculator
                       )