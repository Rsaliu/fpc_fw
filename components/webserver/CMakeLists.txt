cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/webserver.c"
                             "src/login_handler.c"
                            "src/config_handler.c"
                            "src/register_handler.c"
                            "src/credential_store.c"
                            "src/session.c"
                            "src/logout_handler.c"
                            "src/webserver_utils.c"
                            "src/reset_handler.c"
                            "src/auth.c"

                       INCLUDE_DIRS "."
                       REQUIRES common esp_http_server fatfs json spiffs nvs_flash mdns esp_wifi json mbedtls esp_timer
                       PRIV_REQUIRES esp_event
                       )