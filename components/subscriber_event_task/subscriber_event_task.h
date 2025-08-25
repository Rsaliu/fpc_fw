#ifndef __SUBSCRIBER_EVENT_TASK_H__
#define __SUNSCRIBER_EVENT_TASK_H__

#include <stdint.h>
#include <tank_monitor.h>

typedef struct 
{
    tank_monitor_t* monitor;
    tank_monitor_event_hook_t event_hook;
    int event_id;
}subcriber_event_task_t;

void dummy_event_callback(void* context,int actuator_id, event_type_t event,int monitor_id);

void subscriber_event_task(void* Pvparameter);

#endif