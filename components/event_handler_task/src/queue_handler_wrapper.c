#include <stdio.h>
#include <queue_handler_wrapper.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG = "QUEUE_HANDLER_WRAPPER";
QueueHandle_t event_queue;

error_type_t queue_handler_wrapper_send(monitor_event_queue_t* queue_handler){
    if (event_queue == NULL)
    {
        ESP_LOGE(TAG,"event queue is null\n");
        return SYSTEM_NULL_PARAMETER;
    }
 
    if (xQueueSend(event_queue, queue_handler,(TickType_t)0) == pdPASS)
    {
        ESP_LOGI(TAG, "sucessfully send a monitor queue\n");
    
    }else
    {
        ESP_LOGE(TAG, "queue full, resetting...");
        xQueueReset(event_queue);

        if (xQueueSend(event_queue, queue_handler,(TickType_t)0) == pdPASS) {
            ESP_LOGI(TAG, "event sent after queue reset");
        } else {
            ESP_LOGE(TAG, "failed to send even after reset");
            return SYSTEM_FAILED;
        }
    }
  return SYSTEM_OK;
}

error_type_t queue_handler_wrapper_receive(monitor_event_queue_t* queue_handler ){
    if (event_queue == NULL)
    {
        ESP_LOGE(TAG,"queue handler is null\n");
        return SYSTEM_NULL_PARAMETER;
    }

    if (xQueueReceive(event_queue, queue_handler, portMAX_DELAY) == pdPASS)
    {
        ESP_LOGI(TAG, "sucessfully received an event queue from monitors\n");
        return SYSTEM_OK;
     
    }
    ESP_LOGW(TAG, "no event received in 1s");
    
    return SYSTEM_TIMED_OUT;   
}

error_type_t queue_handler_wrapper_init(){

    event_queue = xQueueCreate(10, sizeof(monitor_event_queue_t));
    if (event_queue == NULL)
    {
         ESP_LOGE(TAG, "failed to create event handler queue\n");
         return SYSTEM_NULL_PARAMETER;
    }
    ESP_LOGI(TAG, "event handler queue created successfully");

    return SYSTEM_OK;
}