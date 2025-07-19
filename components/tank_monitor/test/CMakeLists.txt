cmake_minimum_required(VERSION 3.5)

idf_component_register(SRC_DIRS "."
                       INCLUDE_DIRS "."
                       REQUIRES unity
                       PRIV_REQUIRES tank_monitor tank event common cmock)