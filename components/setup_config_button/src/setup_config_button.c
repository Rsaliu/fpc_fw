#include <stdio.h>
#include <setup_config_button.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_FLAG_BIT0 (1 << 0)

static EventGroupHandle_t button_event;
static const char*TAG = "SETUP_CONFIG_BUTTON";

static void dummy_setup_config_button(){
    ESP_LOGI(TAG, "FPC is in configuration mode");

}

static void IRAM_ATTR setup_config_button_isr_handler(void*arg){
    setup_config_button_config_t* config = (setup_config_button_config_t*)arg;
    config->setup_config_button_mode = true;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xEventGroupSetBitsFromISR(button_event, BUTTON_FLAG_BIT0, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    
}

void setup_config_button_task(void* Pvparameter){
    EventBits_t event_bit;
    while (1)
    {
        event_bit = xEventGroupWaitBits(button_event, BUTTON_FLAG_BIT0,pdTRUE,pdFALSE,portMAX_DELAY);
        if ((event_bit & BUTTON_FLAG_BIT0) != 0)
        {
            ESP_LOGI(TAG, "button pressed !!!\n");
             dummy_setup_config_button();
        } 
    }
}

void setup_config_button_init(setup_config_button_config_t* config){
    button_event = xEventGroupCreate();
    if (button_event == NULL)
    {
        ESP_LOGE(TAG, "failed to create event group\n");
    }
    
    gpio_config_t  io_config = {};
    io_config.intr_type = GPIO_INTR_POSEDGE;
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pin_bit_mask = (1ULL << config->button_pin_number);
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_config);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(config->button_pin_number,setup_config_button_isr_handler,(void*) config);
}

