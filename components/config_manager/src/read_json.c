#include "read_json.h"
#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "READ_JSON";

esp_err_t read_config_json(const char *path, char **buffer, size_t *size)
{
    if (path == NULL || buffer == NULL || size == NULL)
    {
        ESP_LOGE(TAG, "Invalid arguments");
        return ESP_ERR_INVALID_ARG;
    }

    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file: %s", path);
        return ESP_FAIL;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *buffer = (char *)malloc(*size + 1);
    if (*buffer == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for file contents");
        fclose(file);
        return ESP_ERR_NO_MEM;
    }

    size_t read_size = fread(*buffer, 1, *size, file);
    if (read_size != *size)
    {
        ESP_LOGE(TAG, "Failed to read file: %s", path);
        free(*buffer);
        *buffer = NULL;
        fclose(file);
        return ESP_FAIL;
    }

    char *data = *buffer;
    data[*size] = '\0';
    fclose(file);
    ESP_LOGI(TAG, "Successfully read file: %s (%zu bytes)", path, *size);
    return ESP_OK;
}