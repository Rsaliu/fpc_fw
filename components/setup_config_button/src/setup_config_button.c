#include <stdio.h>
#include <setup_config_button.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_intr_types.h"
#include "esp_log.h"
#include "webserver_task.h"


#define BUTTON_FLAG_BIT0 (1 << 0) // bit for button press
#define BUTTON_FLAG_BIT1 (1 << 1) // bit for button release
static EventGroupHandle_t button_event;
static const char *TAG = "SETUP_CONFIG_BUTTON";

static void IRAM_ATTR setup_config_button_isr_handler(void *arg)
{
    // 1
    setup_config_button_config_t *config = (setup_config_button_config_t *)arg;
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xHigherPriorityTaskWoken = pdFALSE;

    int level = gpio_get_level(config->button_pin_number);
    EventBits_t bit_to_set = level ? BUTTON_FLAG_PRESS : BUTTON_FLAG_RELEASE;
    xResult = xEventGroupSetBitsFromISR(button_event, BUTTON_FLAG_BIT0, &xHigherPriorityTaskWoken);

    if (xResult != pdFALSE)
    {

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void setup_config_button_task(void *Pvparameter)
{
    setup_config_button_config_t *config = (setup_config_button_config_t *)Pvparameter;

    if (config == NULL)
    {
        ESP_LOGE(TAG, "Null configuration provided");
        vTaskDelete(NULL);
    }

    EventBits_t event_bit;
    while (1)
    {
        event_bit = xEventGroupWaitBits(button_event, BUTTON_FLAG_BIT0, pdTRUE, pdFALSE, portMAX_DELAY);
        if ((event_bit & BUTTON_FLAG_BIT0) != 0)
        {
            ESP_LOGI(TAG, "button pressed !!!\n");
            ///
            if (config->config_button != NULL)
            {
                error_type_t err = config->config_button();
                if (err != SYSTEM_OK)
                {
                    ESP_LOGE(TAG, "configuration failed\n");
                }
            }
            webserver_task_signal_start(); // Signal webserver to start
        }
        else if (event_bit & BUTTON_FLAG_RELEASE)
        {
            ESP_LOGI(TAG, "Button released");
            webserver_task_signal_stop(); // Signal webserver to stop
        }
    }
}

void setup_config_button_init(setup_config_button_config_t *config)
{
    button_event = xEventGroupCreate();
    if (button_event == NULL)
    {
        ESP_LOGE(TAG, "failed to create event group\n");
        return;
    }

    // gpio_config_t io_config = {};
    // io_config.intr_type = GPIO_INTR_POSEDGE;
    // io_config.mode = GPIO_MODE_INPUT;
    // io_config.pin_bit_mask = (1ULL << config->button_pin_number);
    // io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    // gpio_config(&io_config);

    gpio_config_t io_config = {
        io_config.intr_type = GPIO_INTR_POSEDGE,
        io_config.mode = GPIO_MODE_INPUT,
        io_conf0ig.pin_bit_mask = (1ULL << config->button_pin_number),
        io_config.pull_up_en = GPIO_PULLUP_ENABLE,
    };

    gpio_config(&io_config);

    // ESP_INTR_FLAG_LEVEL3 use to set the isr priority level
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    esp_err_t ret = gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install ISR service: %s", esp_err_to_name(ret));
        xEventGroupDelete(button_event);
        return;
    }

    gpio_isr_handler_add(config->button_pin_number, setup_config_button_isr_handler, (void *)config);
    ret = gpio_isr_handler_add(config->button_pin_number, setup_config_button_isr_handler, config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add ISR handler: %s", esp_err_to_name(ret));
        gpio_uninstall_isr_service();
        xEventGroupDelete(button_event);
        return;
    }

    // Create button task
    xTaskCreate(setup_config_button_task, "button_task", 2048, config, 5, NULL);
    ESP_LOGI(TAG, "Button task initialized");
}

