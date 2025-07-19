cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/wifi_hotspot.c"
                       INCLUDE_DIRS "."
                       REQUIRES common esp_wifi nvs_flash
                       )


  