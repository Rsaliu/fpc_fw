
#ifndef __TANK_MONITOR_H__
#define __TANK_MONITOR_H__
#include <common_headers.h>
#include <stdbool.h>
#include "tank.h"
#include "level_sensor.h"
#include <event.h>

// dummy level sensor object
// typedef struct {
//     int id; // Unique identifier for the sensor
// } level_sensor_t;
// // dummy level sensor functions

typedef enum{
    TANK_MONITOR_EVENT_TANK_FULL = 0, // Event when the tank is full
    TANK_MONITOR_EVENT_TANK_LOW = 1, // Event when the tank is below low level
    TANK_MONITOR_EVENT_TANK_NOT_FULL = 2, // Event when the tank is not full
} tank_monitor_event_t;

typedef enum{
    TANK_STATE_MACHINE_NORMAL_STATE = 0, // Normal state of the tank
    TANK_STATE_MACHINE_FULL_STATE = 1, // Tank is full
    TANK_STATE_MACHINE_LOW_STATE = 2, // Tank is below low level
}tank_state_machine_state_t;


typedef struct{
    int id; // Unique identifier for the tank monitor
    tank_t *tank; // Pointer to the tank being monitored
    level_sensor_t *sensor; // Pointer to the level sensor
}tank_monitor_config_t;

typedef enum {
    TANK_MONITOR_NOT_INITIALIZED = 0,
    TANK_MONITOR_INITIALIZED = 1,
} tank_monitor_state_t;

typedef struct tank_monitor_t tank_monitor_t;
typedef void (*tank_monitor_event_callback_t)(void* context,int actuator_id, event_type_t state,int monitor_id);

typedef struct {
    void *context; // Context for the callback, can be used to pass additional data
    int actuator_id; // Action ID for the event
    tank_monitor_event_callback_t callback; // Callback function for the subscriber
} tank_monitor_event_hook_t;

typedef void (*tank_monitor_level_callback_t)(void* context, int tank_monitor_id,uint16_t level_mm); 
typedef struct {
    void *context;
    tank_monitor_level_callback_t callback;
} tank_monitor_level_hook_t;  

tank_monitor_t* tank_monitor_create(tank_monitor_config_t config);
error_type_t tank_monitor_init(tank_monitor_t *monitor);
error_type_t tank_monitor_deinit(tank_monitor_t *monitor);
error_type_t tank_monitor_destroy(tank_monitor_t **monitor);
error_type_t tank_monitor_get_state(const tank_monitor_t *monitor, tank_monitor_state_t *state);
error_type_t tank_monitor_get_config(const tank_monitor_t *monitor, tank_monitor_config_t *config);
error_type_t tank_monitor_check_level(tank_monitor_t *monitor);
error_type_t tank_monitor_subscribe_state_event(tank_monitor_t *monitor, const tank_monitor_event_hook_t* hook,int* event_id);
error_type_t tank_monitor_unsubscribe_state_event(tank_monitor_t *monitor,int event_id);
error_type_t tank_monitor_print_info(tank_monitor_t* monitor);
error_type_t tank_monitor_print_info_into_buffer(tank_monitor_t* monitor, char* buffer, const size_t buffer_size);
error_type_t tank_monitor_subscribe_level_event(tank_monitor_t *monitor, const tank_monitor_level_hook_t *hook,int *event_id);
error_type_t tank_monitor_unsubscribe_level_event(tank_monitor_t *monitor,int event_id);


#endif // __TANK_MONITOR_H__