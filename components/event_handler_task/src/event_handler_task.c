#include <stdio.h>
#include "event_handler_task.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* EVENT = "EVENT_HANDLER";
QueueHandle_t event_queue = NULL;

void event_handler_task(void*Pvparameter){
    event_handler_t evt;
    while (1)
    {
        if (xQueueReceive(event_queue, &evt, portMAX_DELAY))
        {
            ESP_LOGI(EVENT, "received an event queue\n");
            
            switch (evt.event)
            {
            case EVENT_PUMP_OVERCURRENT:
                ESP_LOGI(EVENT,"overcurrent detected\n");
                break;
            case EVENT_PUMP_UNDERCURRENT:
                break;
            case EVENT_PUMP_NORMAL:
                break;
            case EVENT_TANK_FULL_STATE:
                 ESP_LOGI(EVENT,"tank level is full\n");
             break;
            case EVENT_TANK_LOW_STATE:
                 ESP_LOGI(EVENT,"tank level is low\n");
             break;
            case EVENT_TANK_NORMAL_STATE:
                 ESP_LOGI(EVENT,"tank is in normal state\n");
             break;           
            default:
                break;
            }
        }
        
    }
    

}