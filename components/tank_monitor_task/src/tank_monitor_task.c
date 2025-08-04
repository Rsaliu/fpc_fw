#include <tank_monitor_task.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void tank_monitor_task(void* pvParamter){
   tank_monitor_task_config_t* config = (tank_monitor_task_config_t*) pvParamter;
   while (1)
   {
        for (size_t i = 0; i <config->tank_monitor_count; i++)
        {
            if (config->tank_monitor[i])
            {
                tank_monitor_check_level(config->tank_monitor[i]);
            }
            
        }
        vTaskDelay(pdMS_TO_TICKS(500));    
   }
    
}
