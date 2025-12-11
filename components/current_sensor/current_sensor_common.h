#ifndef __CURRENT_SENSOR_COMMON_H__
#define __CURRENT_SENSOR_COMMON_H__
#include <freertos/FreeRTOS.h>
 typedef struct {
    void* context;
    TickType_t timestamp;
    uint8_t channel;
    void* callers_context;
 }overcurrent_queue_item_t;

typedef void (*overcurrent_comparator_callback_t)(overcurrent_queue_item_t item);

#endif // __CURRENT_SENSOR_COMMON_H__