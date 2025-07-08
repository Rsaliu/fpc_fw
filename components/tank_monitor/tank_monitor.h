#ifndef __TANK_MONITOR_H__
#define __TANK_MONITOR_H__
#include <common_headers.h>
#include "tank.h"

// dummy level sensor object
typedef struct {
    int id; // Unique identifier for the sensor
} level_sensor_t;
// dummy level sensor functions

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
    tank_t *tank; // Pointer to the tank being monitored
    level_sensor_t *sensor; // Pointer to the level sensor
    void (*event_callback)(tank_state_machine_state_t state); // Callback for when the tank is full
}tank_monitor_config_t;

typedef enum {
    TANK_MONITOR_NOT_INITIALIZED = 0,
    TANK_MONITOR_INITIALIZED = 1,
} tank_monitor_state_t;

typedef struct tank_monitor_t tank_monitor_t;

tank_monitor_t* tank_monitor_create(tank_monitor_config_t config);
error_type_t tank_monitor_init(tank_monitor_t *monitor);
error_type_t tank_monitor_deinit(tank_monitor_t *monitor);
error_type_t tank_monitor_destroy(tank_monitor_t **monitor);
error_type_t tank_monitor_get_state(tank_monitor_t *monitor, tank_monitor_state_t *state);
error_type_t tank_monitor_get_config(tank_monitor_t *monitor, tank_monitor_config_t *config);
error_type_t tank_monitor_check_level(tank_monitor_t *monitor);

#endif // __TANK_MONITOR_H__