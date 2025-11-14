#ifndef __TANK_MONITOR_TASK_H__
#define __TANK_MONITOR_TASK_H__
#include <stdint.h>
#include <stddef.h>
#include <tank_monitor.h>
#include "common_headers.h"
#include "pump_control_unit.h"
#include "relay_driver.h"


typedef struct 
{
     tank_monitor_t** tank_monitor;
     size_t tank_monitor_count;
     pump_control_unit_t* pcu;
     relay_t** relay;
}tank_monitor_task_config_t;
// note when calling the tank_monitor_task_config_t in main make sure it is a gobal variable
          
void tank_monitor_task(void* pvParameter);
#endif
 