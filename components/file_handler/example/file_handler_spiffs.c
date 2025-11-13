#include "esp_log.h"
#include "esp_err.h"  
#include "esp_spiffs.h"  
#include "file_handler.h"  
#include <string.h>

static const char *TAG = "APP_MAIN";


esp_err_t spiffs_init(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 10,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition info (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}


void spiffs_deinit(void) {
    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "SPIFFS unmounted");
}


void app_main(void) {
    ESP_LOGI(TAG, "Starting application and file system test");

    // Register the FS callbacks
    file_register_fs(spiffs_init, spiffs_deinit);

    // Initialize (mounts SPIFFS via the callback)
    esp_err_t ret = file_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "File initialization failed: %s", esp_err_to_name(ret));
        return;  // Early exit on failure
    }

    // Perform tests here (e.g., write a test file)
    const char *test_data = "Test data";
    size_t data_size = strlen(test_data);
    ret = file_write("/spiffs/test.txt", test_data, data_size);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Test file written successfully");
    } else {
        ESP_LOGE(TAG, "Test write failed: %s", esp_err_to_name(ret));
    }

   
    file_list_files("/spiffs");

    // Cleanup
    file_deinit();  
}