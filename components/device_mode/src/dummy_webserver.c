#include <stdio.h>
#include "dummy_webserver.h"
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

