#ifndef __QUEUE_HANDLER_WRAPPER_H__
#define __QUEUE_HANDLER_WRAPPER_H__

#include <stdint.h>
#include <event.h>
#include <common_headers.h>


typedef struct 
{
    int monitors_id;
    event_type_t event;
}monitor_event_queue_t;

error_type_t queue_handler_wrapper_send(monitor_event_queue_t* queue_handler);
error_type_t queue_handler_wrapper_receive(monitor_event_queue_t* queue_handler );
error_type_t queue_handler_wrapper_init();

#endif
