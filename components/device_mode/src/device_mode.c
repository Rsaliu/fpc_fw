#include <stdio.h>
#include <stdlib.h>
#include <device_mode.h>
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_intr_types.h"
#include "esp_log.h"


static const char*BUTTON_TAG = "SETUP_CONFIG_BUTTON";

static void IRAM_ATTR device_mode_isr_handler(void*arg){
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    esp_restart();
    // if ( xHigherPriorityTaskWoken )
    // {
    //     portYIELD_FROM_ISR();
    // }     
}

error_type_t device_mode_event(device_mode_config_t*config){
    int button_level = gpio_get_level(config->button_pin_number);
    if (button_level == 0)
    {
        ESP_LOGI(BUTTON_TAG, "button pressed: start webserver\n.");
        config->webserver();
        
        
    }else
    {

        ESP_LOGI(BUTTON_TAG, "button release: continue other task");
        config->task();
        
    }

    return SYSTEM_OK;
}

void device_mode_init(device_mode_config_t* config){
    gpio_config_t  io_config = {};
    io_config.intr_type = GPIO_INTR_ANYEDGE;
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pin_bit_mask = (1ULL << config->button_pin_number);
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_config);

    //ESP_INTR_FLAG_LEVEL3 use to set the isr priority level
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    gpio_isr_handler_add(config->button_pin_number,device_mode_isr_handler,(void*) config);
}

