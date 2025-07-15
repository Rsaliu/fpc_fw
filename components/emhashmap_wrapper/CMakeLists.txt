cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/emhashmap/emhashmap.c"
                        INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/emhashmap"
                        )