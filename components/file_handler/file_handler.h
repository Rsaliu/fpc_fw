#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "esp_err.h"
#include "esp_spiffs.h"
#include <sys/stat.h>

typedef esp_err_t (*FileInitFunc)(void);
typedef esp_err_t (*FileDeinitFunc)(void);
void file_register_fs(FileInitFunc init, FileDeinitFunc deinit);

esp_err_t file_init(void);
esp_err_t file_deinit(void);
esp_err_t file_write(const char *path, const char *data, size_t size);
esp_err_t file_read(const char *path, char *buffer, size_t max_size, size_t *read_size);
esp_err_t file_append(const char *path, const char *data, size_t size);
esp_err_t file_rename(const char *src_path, const char *dst_path);
esp_err_t file_delete(const char *path);
esp_err_t file_get_size(const char *path, size_t *size);
void file_list_files(const char *path);

#endif
