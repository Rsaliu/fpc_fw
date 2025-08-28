#ifndef __PUMP_MONITOR_H__
#define __PUMP_MONITOR_H__
#include <common_headers.h>
#include "pump.h"
#include <event.h>
#include <current_sensor.h>

typedef enum
{
    PUMP_MONITOR_EVENT_CURRENT_NORMAL = 0,
    PUMP_MONITOR_EVENT_CURRENT_LOW = 1,
    PUMP_MONITOR_EVENT_OVERCURRENT = 2
} pump_monitor_event_t;

typedef enum
{
    PUMP_STATE_MACHINE_NORMAL_STATE = 0,
    PUMP_STATE_MACHINE_UNDERCURRENT_STATE = 1,
    PUMP_STATE_MACHINE_OVERCURRENT_STATE = 2
} pump_state_machine_state_t;


typedef struct
{
    int id;                 // Unique identifier for the pump monitor
    pump_t *pump;           // Pointer to the pump being monitored
    current_sensor_t *sensor; // Pointer to the current sensor
} pump_monitor_config_t;

typedef enum
{
    PUMP_MONITOR_NOT_INITIALIZED = 0,
    PUMP_MONITOR_INITIALIZED = 1,
} pump_monitor_state_t;


typedef struct pump_monitor_t pump_monitor_t;
typedef void (*pump_monitor_event_callback_t)(void* context,int actuator_id, event_type_t state,int monitor_id);

typedef struct {
    void *context; // Context for the callback, can be used to pass additional data
    int actuator_id; // Action ID for the event
    pump_monitor_event_callback_t callback; // Callback function for the subscriber
} pump_monitor_event_hook_t;

pump_monitor_t* pump_monitor_create(pump_monitor_config_t config);
error_type_t pump_monitor_init(pump_monitor_t *pump_monitor);
error_type_t pump_monitor_deinit(pump_monitor_t   *pump_monitor);
error_type_t pump_monitor_destroy(pump_monitor_t   **pump_monitor);
error_type_t pump_monitor_get_state(const pump_monitor_t   *pump_monitor,  pump_monitor_state_t *state);
error_type_t pump_monitor_get_config(const pump_monitor_t   *pump_monitor,  pump_monitor_config_t *config);
error_type_t pump_monitor_check_current(pump_monitor_t   *pump_monitor);
error_type_t pump_monitor_subscribe_event(pump_monitor_t   *pump_monitor, const pump_monitor_event_hook_t* hook,int* event_id);
error_type_t pump_monitor_unsubscribe_event(pump_monitor_t  *pump_monitor,int event_id);

#endif // __PUMP_MONITOR_H__