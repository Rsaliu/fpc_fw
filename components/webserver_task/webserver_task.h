#ifndef __WEBSERVER_TASK_H__
#define __WEBSERVER_TASK_H__

#include "webserver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "common_headers.h"

error_type_t webserver_task_start(const webserver_config_t* config);
bool webserver_task_is_running(void);

#endif 