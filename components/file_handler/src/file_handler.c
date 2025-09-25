#include "file_handler.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static const char *TAG = "SPIFFS_HANDLER";

esp_err_t spiffs_init(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS...");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 10,  
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition info (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    return ESP_OK;
}

void spiffs_deinit(void)
{
    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "SPIFFS unmounted");
}

esp_err_t spiffs_write_file(const char *path, const char *data)
{
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", path);
        return ESP_FAIL;
    }

    fprintf(f, "%s", data);
    fclose(f);
    ESP_LOGI(TAG, "File written: %s", path);
    return ESP_OK;
}

esp_err_t spiffs_append_file(const char *path, const char *data)
{
    FILE *f = fopen(path, "a");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for appending: %s", path);
        return ESP_FAIL;
    }

    fprintf(f, "%s", data);
    fclose(f);
    ESP_LOGI(TAG, "Data appended to: %s", path);
    return ESP_OK;
}

esp_err_t spiffs_read_file(const char *path, char **buffer, size_t *size)
{
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file: %s", path);
        return ESP_FAIL;
    }

    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);

    *buffer = (char *)malloc(*size + 1);
    if (*buffer == NULL)
    {
        ESP_LOGE(TAG, "Memory allocation failed");
        fclose(f);
        return ESP_ERR_NO_MEM;
    }

    size_t read_size = fread(*buffer, 1, *size, f);
    (*buffer)[read_size] = '\0';
    fclose(f);

    ESP_LOGI(TAG, "File read: %s (%zu bytes)", path, *size);
    return ESP_OK;
}

esp_err_t spiffs_rename_file(const char *src_path, const char *dst_path)
{
    if (rename(src_path, dst_path) != 0)
    {
        ESP_LOGE(TAG, "Failed to rename %s to %s", src_path, dst_path);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "File renamed: %s -> %s", src_path, dst_path);
    return ESP_OK;
}


esp_err_t spiffs_delete_file(const char *path)
{
    if (remove(path) != 0)
    {
        ESP_LOGE(TAG, "Failed to delete file: %s", path);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "File deleted: %s", path);
    return ESP_OK;
}

void spiffs_list_files(void)
{
    DIR *dir = opendir("/spiffs");
    if (dir == NULL)
    {
        ESP_LOGE(TAG, "Failed to open /spiffs directory");
        return;
    }

    struct dirent *entry;
    ESP_LOGI(TAG, "Files in SPIFFS:");
    while ((entry = readdir(dir)) != NULL)
    {
        ESP_LOGI(TAG, "  %s", entry->d_name);
    }
    closedir(dir);
}

esp_err_t spiffs_get_file_size(const char *path, size_t *size)
{
    if (path == NULL || size == NULL) {
        ESP_LOGE(TAG, "Invalid arguments");
        return ESP_ERR_INVALID_ARG;
    }

    struct stat st;
    if (stat(path, &st) == 0) {
        *size = st.st_size;
        ESP_LOGI(TAG, "File size of %s: %ld bytes", path, st.st_size);
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "Failed to get size for: %s", path);
        return ESP_FAIL;
    }
}