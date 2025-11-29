#ifndef __DEVICE_MODE_H__
#define __DEVICE_MODE_H__
#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <common_headers.h>

typedef void(*websever_task_starter_t)(void);
typedef void(*main_tasks_starter_t)(void);

typedef struct 
{
    main_tasks_starter_t task;
    uint8_t button_pin_number;
    websever_task_starter_t webserver;
}device_mode_config_t;

error_type_t device_mode_event(const device_mode_config_t*config);
error_type_t device_mode_init(const device_mode_config_t *config);

 
#endif