#ifndef __WEBSERVER_TASK_H__
#define __WEBSERVER_TASK_H__

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "common_headers.h"
#include "webserver.h"
#include "esp_err.h"
#include "esp_vfs.h"

typedef struct {
    webserver_config_t webserver_config; 
} webserver_task_config_t;

esp_err_t webserver_task_init(webserver_task_config_t *config);
//error_type_t webserver_task_init(webserver_task_config_t *config);
esp_err_t webserver_task_start(void);
//error_type_t webserver_task_start(void);
void webserver_task_signal_start(void);
void webserver_task_signal_stop(void);

#endif 