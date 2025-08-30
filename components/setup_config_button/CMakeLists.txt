cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/setup_config_button.c" 
                       INCLUDE_DIRS "."
                       REQUIRES sp_driver_gpio common
                       )