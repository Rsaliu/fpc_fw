#include <tank_monitor_task.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"


static const char*TAG = "TANK_MONITOR_TASK";

void tank_monitor_task(void* pvParameter){
   tank_monitor_task_config_t* config = (tank_monitor_task_config_t*) pvParameter;

   if (config == NULL ||config->tank_monitor == NULL)
   {
      ESP_LOGE(TAG, "Invalide config or invalide tank monitor array");
      exit(1);
   }

     monitor_event_queue_t tank_monitor_event = {
     .monitors_id = 1,
     .event = EVENT_TANK_NORMAL_STATE

   };
   
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

           error_type_t err = queue_handler_wrapper_send(&tank_monitor_event);
           
           if ( err != SYSTEM_OK)
           {
               ESP_LOGE(TAG, "failed to send tank monitor event\n.");
           }
           
      }
      vTaskDelay(pdMS_TO_TICKS(1000));  
  }

}
