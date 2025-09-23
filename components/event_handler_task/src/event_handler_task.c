#include <stdio.h>
#include <event_handler_task.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char* TAG = "EVENT_HANDLER";

QueueHandle_t event_handler;

void event_handler_task(void*Pvparameter){
    event_handler_t evt;
    while (1)
    {
        if (xQueueReceive(event_handler, &evt, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "received tank monitor task queue\n");
            switch (evt.event)
            {
            case EVENT_PUMP_OVERCURRENT:
                ESP_LOGI(TAG,"overcurrent detected\n");
                break;
            case EVENT_PUMP_UNDERCURRENT:
                break;
            case EVENT_PUMP_NORMAL:
             break;
            case EVENT_TANK_FULL_STATE:
                 ESP_LOGI(TAG,"tank level is full\n");
             break;
            case EVENT_TANK_LOW_STATE:
                 ESP_LOGI(TAG,"tank level is low\n");
             break;
            case EVENT_TANK_NORMAL_STATE:
                 ESP_LOGI(TAG,"tank is in normal state\n");
             break;           
            default:
                break;
            }
        }
        
    }
    

}