#include "device_mode.h"
#include "dummy_webserver.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static void dummy_webserver(void* pvParameter){
    for (;;)
    {
        ESP_LOGI("DUMMY_WEBSERVER", "FPC is in configuration mode");
        vTaskDelay(1000);
    }   
}

void create_dummy_webserver_task(void){

    xTaskCreate(&dummy_webserver, "websever_task",4096,NULL,1,NULL);
}

device_mode_config_t config = {
    .main_tasks_starter_t = task_handler,
    .button_pin_number = 4,
    .websever_task_starter_t = create_dummy_webserver_task
};

void app_main(void)
{
    printf("Hello world!\n");
    error_type_t err =  device_mode_init(&config);
    if(err != SYSTEM_OK){
        ESP_LOGE("DEVICE_MODE", "Device mode init failed");
        return;
    }
    err = device_mode_event(&config);
    if(err != SYSTEM_OK){
        ESP_LOGE("DEVICE_MODE", "Device mode event handling failed");
        return;
    }
}