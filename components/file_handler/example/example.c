#include "file_handler.h"
#include "esp_log.h"
#include <string.h>  // For strlen
#include <stdlib.h>  // For malloc/free
static const char *TAG = "APP_MAIN";

void app_main(void)
{
    // Initialize SPIFFS
    if (file_init() != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS init failed");
        return;
    }

    // Write file
    const char *write_data = "{ \"message\": \"Hello ESP32!\" }\n";
    size_t write_size = strlen(write_data);
    if (file_write("/spiffs/test.json", write_data, write_size) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write file");
        file_deinit();
        return;
    }

    // Get file Size
    size_t file_size = 0;
    if (file_get_size("/spiffs/test.json", &file_size) == ESP_OK) {
        ESP_LOGI(TAG, "File size after write: %zu bytes", file_size);
    }

    // add to the file
    const char *append_data = "{ \"extra\": \"Appending new line...\" }\n";
    size_t append_size = strlen(append_data);
    if (file_append("/spiffs/test.json", append_data, append_size) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to append to file");
        file_deinit();
        return;
    }

    // Get file size again
    if (file_get_size("/spiffs/test.json", &file_size) == ESP_OK) {
        ESP_LOGI(TAG, "File size after append: %zu bytes", file_size);
    }

    // Read file
    // First, get size to allocate buffer
    if (file_get_size("/spiffs/test.json", &file_size) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get file size for read");
        file_deinit();
        return;
    }
    char *buffer = NULL;
    if (file_size > 0) {
        buffer = malloc(file_size + 1);
        if (buffer == NULL) {
            ESP_LOGE(TAG, "Memory allocation failed for read buffer");
            file_deinit();
            return;
        }
        size_t read_size = 0;
        if (file_read("/spiffs/test.json", buffer, file_size + 1, &read_size) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read file");
            free(buffer);
            file_deinit();
            return;
        }
        // Null-terminate if needed (already done in file_read)
        buffer[read_size] = '\0';
    } else {
        buffer = malloc(1);  // For empty string
        if (buffer) *buffer = '\0';
    }
    ESP_LOGI(TAG, "File contents:\n%s", buffer ? buffer : "");
    free(buffer);

    // Rename file
    if (file_rename("/spiffs/test.json", "/spiffs/test_renamed.json") != ESP_OK) {
        ESP_LOGE(TAG, "Failed to rename file");
        file_deinit();
        return;
    }

    // List files
    file_list_files("/spiffs");

    // Delete file
    if (file_delete("/spiffs/test_renamed.json") != ESP_OK) {
        ESP_LOGE(TAG, "Failed to delete file");
    }

    // Deinit
    file_deinit();
}
