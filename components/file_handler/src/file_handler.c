#include "file_handler.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static const char *TAG = "FILE_HANDLER";

static FileInitFunc init_func = NULL;
static FileDeinitFunc deinit_func = NULL;

void file_register_fs(FileInitFunc init, FileDeinitFunc deinit) {
    init_func = init;
    deinit_func = deinit;
}

esp_err_t file_init(void) {
    ESP_LOGI(TAG, "Initializing FILE...");
    if (init_func == NULL) {
        ESP_LOGE(TAG, "No init function registered");
        return ESP_FAIL;
    }
    return init_func();
}

esp_err_t file_deinit(void) {
    if (deinit_func != NULL) {
         return deinit_func();
    } else {
        ESP_LOGE(TAG, "No deinit function registered");
        return ESP_FAIL;
    }

}

esp_err_t file_write(const char *path, const char *data, size_t size)
{
    if (path == NULL || data == NULL || size == 0) {
        ESP_LOGE(TAG, "Invalid params for write: %s (size=%zu)", path ? path : "NULL", size);
        return ESP_FAIL;
    }

    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", path);
        return ESP_FAIL;
    }

    size_t written = fwrite(data, 1, size, f);  
    if (written != size) {
        ESP_LOGE(TAG, "Incomplete write to %s: %zu/%zu bytes", path, written, size);
        fclose(f);
        return ESP_FAIL;
    }

    fflush(f);  // Ensure data is flushed before closing
    fclose(f);
    ESP_LOGI(TAG, "File written: %s (%zu bytes)", path, size);
    return ESP_OK;
}

esp_err_t file_append(const char *path, const char *data, size_t size)
{
    if (path == NULL || data == NULL || size == 0) {
        ESP_LOGE(TAG, "Invalid params for append: %s (size=%zu)", path ? path : "NULL", size);
        return ESP_FAIL;
    }

    FILE *f = fopen(path, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for appending: %s", path);
        return ESP_FAIL;
    }

    size_t written = fwrite(data, 1, size, f);  
    if (written != size) {
        ESP_LOGE(TAG, "Incomplete append to %s: %zu/%zu bytes", path, written, size);
        fclose(f);
        return ESP_FAIL;
    }

    fflush(f);  
    fclose(f);
    ESP_LOGI(TAG, "Data appended to: %s (%zu bytes)", path, size);
    return ESP_OK;
}

esp_err_t file_read(const char *path, char *buffer, size_t max_size, size_t *read_size)
{
    if (path == NULL || buffer == NULL || max_size == 0 || read_size == NULL) {
        ESP_LOGE(TAG, "Invalid params for read: %s (max_size=%zu)", path ? path : "NULL", max_size);
        return ESP_FAIL;
    }

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file: %s", path);
        return ESP_FAIL;
    }

    // Get file size
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size == 0) {  // Empty file
        fclose(f);
        *buffer = '\0';  // Null-terminate empty buffer
        *read_size = 0;
        ESP_LOGI(TAG, "Empty file read: %s", path);
        return ESP_OK;
    }

    // Cap read size to available buffer space (leave room for null-terminator)
    size_t to_read = (file_size < max_size - 1) ? file_size : max_size - 1;
    size_t actual_read = fread(buffer, 1, to_read, f);

    if (actual_read != to_read) {
        ESP_LOGE(TAG, "Incomplete read from %s: %zu/%zu bytes", path, actual_read, to_read);
        fclose(f);
        return ESP_FAIL;
    }

    buffer[actual_read] = '\0';  // Null-terminate
    *read_size = actual_read;
    fclose(f);

    ESP_LOGI(TAG, "File read: %s (%zu bytes)", path, actual_read);
    if (file_size > max_size - 1) {
        ESP_LOGW(TAG, "File %s truncated: %zu/%zu bytes read", path, actual_read, file_size);
    }
    return ESP_OK;
}

esp_err_t file_rename(const char *src_path, const char *dst_path)
{
    if (rename(src_path, dst_path) != 0)
    {
        ESP_LOGE(TAG, "Failed to rename %s to %s", src_path, dst_path);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "File renamed: %s -> %s", src_path, dst_path);
    return ESP_OK;
}


esp_err_t file_delete(const char *path)
{
    if (remove(path) != 0)
    {
        ESP_LOGE(TAG, "Failed to delete file: %s", path);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "File deleted: %s", path);
    return ESP_OK;
}

void file_list_files(const char *path)
{
    if (path == NULL) {
        ESP_LOGE(TAG, "Invalid path provided (NULL)");
        return;
    }

    DIR *dir = opendir(path);
    if (dir == NULL) {
        ESP_LOGE(TAG, "Failed to open directory: %s", path);
        return;
    }

    struct dirent *entry;
    ESP_LOGI(TAG, "Files in %s:", path);
    while ((entry = readdir(dir)) != NULL) {
        ESP_LOGI(TAG, "  %s", entry->d_name);
    }
    closedir(dir);
}

esp_err_t file_get_size(const char *path, size_t *size)
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