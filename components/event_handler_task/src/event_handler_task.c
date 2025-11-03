#include <stdio.h>
#include "event_handler_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "common_headers.h"
#include "queue_handler_wrapper.h"

static const char *TAG = "EVENT_HANDLER";

void event_handler_task(void *Pvparameter)
{
    monitor_event_queue_t event_handler;
    while (1)
    {
        error_type_t err = queue_handler_wrapper_receive(&event_handler);
        if (err != SYSTEM_OK)
        {
            ESP_LOGE(TAG, "failed to receive a queue\n.");
        }
        else
        {
            switch (event_handler.event)
            {
            case EVENT_PUMP_OVERCURRENT:
                ESP_LOGI(TAG, "overcurrent detected\n");
                break;
            case EVENT_PUMP_UNDERCURRENT:
                break;
            case EVENT_PUMP_NORMAL:
                break;
            case EVENT_TANK_FULL_STATE:
                ESP_LOGI(TAG, "tank level is full\n");
                break;
            case EVENT_TANK_LOW_STATE:
                ESP_LOGI(TAG, "tank level is low\n");
                break;
            case EVENT_TANK_NORMAL_STATE:
                ESP_LOGI(TAG, "tank is in normal state\n");
                break;
            default:
                break;
            }
        }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
}
