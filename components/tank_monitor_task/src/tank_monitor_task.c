#include <tank_monitor_task.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char*TAG = "TANK_MONITOR_TASK";

extern QueueHandle_t event_handler;

void tank_monitor_task(void* pvParameter){
   tank_monitor_task_config_t* config = (tank_monitor_task_config_t*) pvParameter;
   tank_monitor_event_handler_t monitor_event;
   
   
   if (config == NULL ||
    
    config->tank_monitor == NULL)
   {
      ESP_LOGE(TAG, "Invalide config or invalide tank monitor array");
      exit(1);
   }
   
   ESP_LOGI(TAG,"entering the task monitor loop......\n");
   while (1)
   {
        for (int i = 0; i <config->tank_monitor_count; i++)
        {
            if (config->tank_monitor[i] == NULL) {
                ESP_LOGW(TAG, "Monitor[%d] is NULL", i);
            }
            ESP_LOGI(TAG, "number of monitor[%d]\n", i);
           tank_monitor_check_level(config->tank_monitor[i]);   
           xQueueSend(event_handler, &monitor_event,(TickType_t)0);

      }
      vTaskDelay(pdMS_TO_TICKS(1000));  
  }

}
