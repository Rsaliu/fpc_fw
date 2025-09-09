cmake_minimum_required(VERSION 3.5)

idf_component_register(SRC_DIRS "."
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES pump_monitor_task common)

