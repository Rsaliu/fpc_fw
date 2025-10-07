// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <esp_log.h>
// #include "setup_config_button.h"
// #include "webserver_task.h"
// #include "nvs_flash.h"

// static const char *TAG = "MAIN";

// void app_main(void) {
//     // Initialize NVS (required by webserver)
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

//     // Initialize WiFi (must be in AP or APSTA mode; implement as needed)
//     // e.g., wifi_init_ap();

//     // Setup button config
//     setup_config_button_config_t button_cfg = {
//         .button_pin_number = 0, // Replace with actual GPIO pin
//         .config_button = NULL   // Optional callback
//     };
//     setup_config_button_init(&button_cfg);

//     // Setup webserver task config
//     webserver_task_config_t webserver_task_cfg = {
//         .webserver_cfg = {
//             .port = 80,
//             .document_root = "/www",
//             .max_connections = 5,
//             .mdns_instance = "esp_webserver",
//             .mdns_hostname = "esp32",
//             .base_path = "/",
//             .web_mount_point = "/www",
//             .web_partition_label = "www",
//             .config_file_path = "/www/config.json"
//         }
//     };

//     // Initialize and start webserver task
//     ESP_ERROR_CHECK(webserver_task_init(&webserver_task_cfg));
//     ESP_ERROR_CHECK(webserver_task_start());

//     ESP_LOGI(TAG, "System initialized successfully");
// }


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "setup_config_button.h"
#include "webserver_task.h"
#include "nvs_flash.h"

static const char *TAG = "MAIN";

void app_main(void) {
    // Initialize NVS (required by webserver)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi (must be in AP or APSTA mode; implement as needed)
    // e.g., wifi_init_ap();

    // Setup button config
    setup_config_button_config_t button_cfg = {
        .button_pin_number = 0, // Replace with actual GPIO pin
        .config_button = NULL   // Optional callback
    };
    setup_config_button_init(&button_cfg);

    // Setup webserver task config
    webserver_task_config_t webserver_task_cfg = {
        .webserver_cfg = {
            .port = 80,
            .document_root = "/www",
            .max_connections = 5,
            .mdns_instance = "esp_webserver",
            .mdns_hostname = "esp32",
            .base_path = "/",
            .web_mount_point = "/www",
            .web_partition_label = "www",
            .config_file_path = "/www/config.json"
        }
    };

    // Initialize and start webserver task
    ESP_ERROR_CHECK(webserver_task_init(&webserver_task_cfg));
    ESP_ERROR_CHECK(webserver_task_start());

    ESP_LOGI(TAG, "System initialized successfully");
}