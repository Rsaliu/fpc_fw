#ifndef __TANK_MONITOR_TASK_H__
#define __TANK_MONITOR_TASK_H__
#include <stdint.h>
#include <stddef.h>
#include <tank_monitor.h>

typedef struct 
{
    tank_monitor_t** tank_monitor;
    size_t tank_monitor_count;
}tank_monitor_task_config_t;


void tank_monitor_task(void* pvParamter);
#endif
