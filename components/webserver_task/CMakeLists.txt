# cmake_minimum_required(VERSION 3.5)

# include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# project(your_project_name)

# set(SOURCES
#     main.c
#     setup_config_button.c
#     webserver.c
#     webserver_task.c
    
# )

idf_component_register(SRCS ${SOURCES}
                      INCLUDE_DIRS "."
                      REQUIRES freertos esp_http_server esp_wifi nvs_flash mdns esp_vfs esp_spiffs driver
                    
)















# cmake_minimum_required(VERSION 3.5)

# idf_component_register(SRCS "src/webserver_task.c" 
#                        INCLUDE_DIRS "."
#                        REQUIRES common freertos esp_http_server esp_wifi nvs_flash mdns esp_vfs esp_spiffs driver webserver
#                        )