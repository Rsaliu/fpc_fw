#ifndef __EVENT_HANDLER_TASK_H__
#define __EVENT_HANDLER_TASK_H__

#include <stdint.h>
#include <stdlib.h>
#include "event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

extern QueueHandle_t event_queue;
typedef struct 
{
   event_type_t event;
}event_handler_t;

void event_handler_task(void*Pvparameter);
#endif