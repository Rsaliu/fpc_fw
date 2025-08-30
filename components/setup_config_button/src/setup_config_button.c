#include <stdio.h>
#include <setup_config_button.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

static QueueHandle_t setup_config_button_event = NULL;

static void IRAM_ATTER setup_config_button_isr_handler(void*arg){
    setup_config_button_config_t* config = (setup_config_button_config_t*)arg;
    int pin_num = config->button_pin_number;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(setup_config_button_event,&pin_num, xHigherPriorityTaskWoken);
}
