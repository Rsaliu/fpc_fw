#include "esp_log.h"
#include "esp_err.h"  // For esp_err_t and esp_err_to_name
#include "esp_vfs_fat.h"  // For FATFS config
#include "sdmmc_cmd.h"    // For sdmmc_card_t
#include "driver/sdmmc_host.h"  // For SDMMC host
#include "file_handler.h"  // Your abstracted handler
#include <string.h>

static const char *TAG = "APP_MAIN";

static sdmmc_card_t *card;  // Global pointer for card info (needed for unmount)

esp_err_t fatfs_sd_init(void) {
    esp_err_t ret;

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,  // Format if mount fails
        .max_files = 10,
        .allocation_unit_size = 16 * 1024
    };

    ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ret;
    }

    // Log card info (analogous to spiffs_info)
    sdmmc_card_print_info(stdout, card);
    ESP_LOGI(TAG, "FATFS mounted at /sdcard");

    return ESP_OK;
}

void fatfs_sd_deinit(void) {
    esp_vfs_fat_sdmmc_unmount();
    ESP_LOGI(TAG, "FATFS unmounted");
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting application and file system test");

    // Register the FS callbacks
    file_register_fs(fatfs_sd_init, fatfs_sd_deinit);

    // Initialize (mounts FATFS via the callback)
    esp_err_t ret = file_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "File initialization failed: %s", esp_err_to_name(ret));
        return;  // Early exit on failure
    }

    // Perform tests here (e.g., write a test file)
    const char *test_data = "Test data";
    size_t data_size = strlen(test_data);
    ret = file_write("/sdcard/test.txt", test_data, data_size);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Test file written successfully");
    } else {
        ESP_LOGE(TAG, "Test write failed: %s", esp_err_to_name(ret));
    }

    // List files for verification
    file_list_files("/sdcard");

    // Cleanup
    file_deinit();  // Calls fatfs_sd_deinit via callback
}


