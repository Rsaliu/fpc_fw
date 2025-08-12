cmake_minimum_required(VERSION 3.5)
idf_component_register(SRCS "src/config_manager.c"
                      INCLUDE_DIRS "."
                      REQUIRES common
                      PRIV_REQUIRES json spiffs
                      )

                     
