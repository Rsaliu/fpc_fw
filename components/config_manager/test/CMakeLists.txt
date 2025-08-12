cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "test_config_manager.c"
                      INCLUDE_DIRS "."
                      REQUIRES unity config_manager
                      PRIV_REQUIRES json spiffs
                      )