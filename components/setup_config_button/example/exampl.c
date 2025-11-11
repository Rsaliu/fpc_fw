#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <setup_config_button.h>
#include "esp_log.h"

static const char*TAG = "SETUP_CONFIG_BUTTON";
static void dummy_webserver(void*pvParameter){
    for (;;)
    {
        ESP_LOGI(TAG, "FPC is in configuration mode");
        vTaskDelay(1000);
    }
    
    
}
 TaskHandle_t other_task_handle;
setup_config_button_config_t button_config = {
    .button_pin_number = 4,
    .other_task = NULL,
    .webserver = dummy_webserver
};

void app_main(void)
{

    printf("Hello world!\n");

        
         setup_config_button_init(&button_config);
         setup_config_button_two_event(&button_config);
}         