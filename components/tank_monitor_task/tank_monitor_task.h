#ifndef __TANK_MONITOR_TASK_H__
#define __TANK_MONITOR_TASK_H__
#include <stdint.h>
#include <stddef.h>
#include <tank_monitor.h>
#include "event.h"

typedef struct 
{
      tank_monitor_t** tank_monitor;
    size_t tank_monitor_count;
}tank_monitor_task_config_t;
// note when calling the tank_monitor_task_config_t in main make sure it is a gobal variable

typedef struct 
{
    int tank_monitor_id;
    int tank_monitor_event;
}tank_monitor_event_handler_t;


void tank_monitor_task(void* pvParameter);
#endif
