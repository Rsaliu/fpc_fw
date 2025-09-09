#ifndef READ_JSON_H
#define READ_JSON_H

#include <esp_err.h>
#include <stdio.h>
esp_err_t read_config_json(const char *path, char **buffer, size_t *size);

#endif 