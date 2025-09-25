#include "file_handler.h"
#include "esp_log.h"

static const char *TAG = "APP_MAIN";

void app_main(void)
{
    // Initialize SPIFFS
    if (spiffs_init() != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS init failed");
        return;
    }

    // Write file
    spiffs_write_file("/spiffs/test.json", "{ \"message\": \"Hello ESP32!\" }\n");

    // Get file Size
    size_t file_size = 0;
    if (spiffs_get_file_size("/spiffs/test.json", &file_size) == ESP_OK) {
        ESP_LOGI(TAG, "File size after write: %zu bytes", file_size);
    }

    // add to the file
    spiffs_append_file("/spiffs/test.json", "{ \"extra\": \"Appending new line...\" }\n");

    // Get file size again
    if (spiffs_get_file_size("/spiffs/test.json", &file_size) == ESP_OK) {
        ESP_LOGI(TAG, "File size after append: %zu bytes", file_size);
    }

    // Read file
    char *buffer = NULL;
    size_t size = 0;
    if (spiffs_read_file("/spiffs/test.json", &buffer, &size) == ESP_OK) {
        ESP_LOGI(TAG, "File contents:\n%s", buffer);
        free(buffer);
    }

    // Rename file
    spiffs_rename_file("/spiffs/test.json", "/spiffs/test_renamed.json");

    // List files
    spiffs_list_files();

    // Delete file
    spiffs_delete_file("/spiffs/test_renamed.json");

    // Deinit
    spiffs_deinit();
}
