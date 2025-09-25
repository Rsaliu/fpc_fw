#ifndef __PUMP_MONITOR_TASK_H__
#define __PUMP_MONITOR_TASK_H__

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cJSON.h>
#include <string.h>
#include <stdlib.h>
#include "pump.h"
#include "current_sensor.h"
#include "pump_monitor.h"
#include "adc_reader.h"
#include "acs712_current_sensor.h"
#include "ads1115.h"
#include "current_sensor_context.h"

typedef struct
{
    int pump_monitor_id;
    int pump_monitor_event;
}pump_monitor_event_handler_t;



esp_err_t start_pump_monitor_task(const char *json_str, size_t json_size, TaskHandle_t *task_handle);


#endif 

