cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/webserver.c"
                       INCLUDE_DIRS "."
                       REQUIRES common esp_http_server fatfs json spiffs nvs_flash mdns esp_wifi
                       PRIV_REQUIRES esp_event
                       )