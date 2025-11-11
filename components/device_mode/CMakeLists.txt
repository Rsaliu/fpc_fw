cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/device_mode.c" "src/example.c" "src/dummy_webserver.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_gpio common tank_monitor_task level_sensor protocol rs485 tank tank_monitor
                       )