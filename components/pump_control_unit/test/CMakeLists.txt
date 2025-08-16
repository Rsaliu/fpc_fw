cmake_minimum_required(VERSION 3.5)

idf_component_register(SRC_DIRS "."
                       INCLUDE_DIRS "."
                       REQUIRES unity
                       PRIV_REQUIRES pump_control_unit tank_monitor level_sensor rs485 protocol event common cmock)
