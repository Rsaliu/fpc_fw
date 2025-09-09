#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <setup_config_button.h>
#include <common_headers.h>
#include "esp_log.h"

static const char*TAG = "SETUP_CONFIG_BUTTON";

static error_type_t dummy_setup_config_button(){
    ESP_LOGI(TAG, "FPC is in configuration mode");
    return SYSTEM_OK;

}

setup_config_button_config_t config = {
    .button_pin_number = 4,
    .config_button = dummy_setup_config_button
};

void app_main(void)
{
    printf("Hello world!\n");
    setup_config_button_init(&config);
    xTaskCreate(&setup_config_button_task, "setup_config_button", 2048, &config, 1, NULL);
}    