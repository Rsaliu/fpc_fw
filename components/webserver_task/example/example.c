#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "nvs_flash.h"       
#include "esp_netif.h"       
#include "esp_event.h"       
#include "esp_wifi.h" 
#include "esp_log.h"       
#include "webserver_task.h"  

static const char *TAG = "WEBSERVER_TASK";

void app_main(void)
{
    printf("Hello world!\n");

    
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    printf("NVS initialized successfully\n");

   
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    printf("Network stack and event loop initialized\n");

 
    esp_netif_create_default_wifi_ap();


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32-WebServer",  
            .ssid_len = strlen("ESP32-WebServer"),
            .channel = 1,               
            .password = "password123",  
            .max_connection = 4,       
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    if (strlen("password123") == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started. SSID: ESP32-WebServer, Password: password123\n");
    ESP_LOGI(TAG, "Connect to this AP and visit http://192.168.4.1/ or http://esp32-webserver.local/\n");

    
    webserver_config_t config = {
        .port = 80,                          // HTTP port
        .document_root = "/www",             // Root for serving static files
        .max_connections = 4,                // Max concurrent connections
        .mdns_instance = "ESP32-WebServer",  // mDNS service instance
        .mdns_hostname = "esp32-webserver",  // mDNS hostname
        .base_path = "/",                    // Base URI path
        .web_mount_point = "/www",           // SPIFFS mount point
        .web_partition_label = "spiffs",     // Matches your partition table label
        .config_file_path = "/www/config.json"  // Example config file (create in SPIFFS if needed)
    };

    // NEW: Start the webserver in a dedicated FreeRTOS task
    error_type_t err = webserver_task_start(&config);
    if (err != SYSTEM_OK) {
        ESP_LOGE(TAG, "Failed to start webserver task: %d\n", err);
        return;
    }

    ESP_LOGI(TAG, "Webserver task started. Check: %s\n", webserver_task_is_running() ? "true" : "false");

    
    int counter = 0;
    while (1) {
        ESP_LOGI(TAG, "Main task running... (uptime: %d seconds)\n", ++counter);
        vTaskDelay(5000 / portTICK_PERIOD_MS);  
    }
}