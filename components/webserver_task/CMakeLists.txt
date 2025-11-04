
cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/webserver_task.c"
                       INCLUDE_DIRS "."
                        REQUIRES freertos esp_http_server esp_wifi nvs_flash mdns vfs common webserver
                       )
