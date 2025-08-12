#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <esp_err.h>
#include <stdio.h>


esp_err_t read_config_json(const char *path, char **buffer, size_t *size);
esp_err_t validate_config_json(const char *path);
#endif 