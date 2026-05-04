#ifndef __PUMP_MONITOR_H__
#define __PUMP_MONITOR_H__

#include <stdbool.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <stdlib.h>
#include <event.h>
#include "pump.h"
#include "current_sensor.h"
#include "adc_reader.h"
#include "acs712_current_sensor.h"
#include "ads1115.h"
#include "current_sensor_context.h"
#include <current_sensor_common.h>

// For now pump_monitor only supports continuous monitoring mode, other modes can be added later

typedef error_type_t (*current_value_read_callback_t)(void* context,float* current_value);
typedef error_type_t (*current_sensor_continuous_read_callback_t)(void* sensor, measurement_complete_callback_t callback, void* context);
typedef enum
{
    PUMP_STATE_MACHINE_NORMAL_STATE = 0,
    PUMP_STATE_MACHINE_UNDERCURRENT_STATE = 1,
    PUMP_STATE_MACHINE_OVERCURRENT_STATE = 2
} pump_state_machine_state_t;
typedef error_type_t (*current_analytics_callback_t)(float* sampled_current_values, int number_of_samples, float rated_current, float min_working_current, pump_state_machine_state_t* state);

typedef struct
{
    int id;                 // Unique identifier for the pump monitor
    pump_t *pump;           // Pointer to the pump being monitored
    current_sensor_t *sensor; // Pointer to the current sensor
    void* current_read_cb; // Callback function to read current val
    int number_of_samples_for_average; // Number of samples to average for current reading (if applicable)
    current_analytics_callback_t analytics_cb; // Callback function for current analytics (if applicable)
} pump_monitor_config_t;

typedef enum
{
    PUMP_MONITOR_NOT_INITIALIZED = 0,
    PUMP_MONITOR_INITIALIZED = 1,
} pump_monitor_state_t;


typedef struct pump_monitor_t pump_monitor_t;
typedef void (*pump_monitor_event_callback_t)(void* context, event_type_t state,int monitor_id);

// typedef struct {
//     void *context; // Context for the callback, can be used to pass additional data
//     //int actuator_id; // Action ID for the event
//     pump_monitor_event_callback_t callback; // Callback function for the subscriber
// } pump_monitor_event_hook_t;

typedef struct
{
    int id;                          // Unique identifier for the subscriber
    void *context; // Context for the callback, can be used to pass additional data
    //pump_monitor_event_hook_t hook;  // Hook stored inline (safer)
    pump_monitor_event_callback_t callback; // Callback function for the subscrib
    bool in_use;                     // whether this slot is occupied
} pump_monitor_subscriber_t;

typedef struct pump_monitor_t pump_monitor_t;

pump_monitor_t* pump_monitor_create(pump_monitor_config_t config);
error_type_t pump_monitor_init(pump_monitor_t *pump_monitor);
error_type_t pump_monitor_deinit(pump_monitor_t   *pump_monitor);
error_type_t pump_monitor_destroy(pump_monitor_t   **pump_monitor);
error_type_t pump_monitor_check_current(pump_monitor_t   *pump_monitor);


error_type_t pump_monitor_get_state(const pump_monitor_t   *pump_monitor,  pump_monitor_state_t *state);
error_type_t pump_monitor_get_config(const pump_monitor_t   *pump_monitor,  pump_monitor_config_t *config);


// the event_id returned can be used to unsubscribe later, it is an index to the subscriber slot
error_type_t pump_monitor_subscribe_event(pump_monitor_t   *pump_monitor, const pump_monitor_subscriber_t* subscriber,int* event_id);
error_type_t pump_monitor_unsubscribe_event(pump_monitor_t  *pump_monitor,int event_id);

#endif // __PUMP_MONITOR_H__
