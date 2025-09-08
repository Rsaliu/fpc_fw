#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <setup_config_button.h>


setup_config_button_config_t config = {
    .button_pin_number = 4,
    .setup_config_button_mode = false
};

void app_main(void)
{
    printf("Hello world!\n");
    setup_config_button_init(&config);
    xTaskCreate(&setup_config_button_task, "setup_config_button", 2048, &config, 1, NULL);
}    