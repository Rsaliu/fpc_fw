cmake_minimum_required(VERSION 3.5)

idf_component_register(SRC_DIRS "."
                       INCLUDE_DIRS "."
                       REQUIRES 
                       PRIV_REQUIRES tank_monitor_task common)