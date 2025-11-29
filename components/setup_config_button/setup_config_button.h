#ifndef __SETUP_CONFIG_BUTTON_H__
#define __SETUP_CONFIG_BUTTON_H__
#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <common_headers.h>

typedef void(*websever_task_t)(void*pvParameter);

typedef struct 
{

    TaskHandle_t other_task;
    uint8_t button_pin_number;
    websever_task_t webserver;
}setup_config_button_config_t;

error_type_t setup_config_button_two_event(setup_config_button_config_t*config);
void setup_config_button_init(setup_config_button_config_t* config);


#endif