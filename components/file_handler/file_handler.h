#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "esp_err.h"
#include "esp_spiffs.h"
#include <sys/stat.h>


esp_err_t spiffs_init(void);
void spiffs_deinit(void);
esp_err_t spiffs_write_file(const char *path, const char *data);
esp_err_t spiffs_read_file(const char *path, char **buffer, size_t *size);
esp_err_t spiffs_append_file(const char *path, const char *data);
esp_err_t spiffs_rename_file(const char *src_path, const char *dst_path);
esp_err_t spiffs_delete_file(const char *path);
esp_err_t spiffs_get_file_size(const char *path, size_t *size);
void spiffs_list_files(void);

#endif
