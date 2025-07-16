#ifndef __pump_control_unit_H__
#define __pump_control_unit_H__
#include <common_headers.h>
#include <tank_monitor.h>
#include <pump.h>
#include <event.h>
#include <stdbool.h>
#include <emhashmap.h>
#include <relay_driver.h>



//dummy current sensor object
typedef struct {
    int id; // Unique identifier for the sensor
} current_sensor_t;

// dummy pump monitor object
typedef struct {
    int id; // Unique identifier for the pump monitor
    pump_t* pump;
    current_sensor_t* current_sensor; // Pointer to the current sensor
}pump_monitor_t;

// dummy pump monitor configuration object
typedef struct{
    int id; // Unique identifier for the tank monitor
    pump_t *pump; // Pointer to the tank being monitored
    current_sensor_t *current_sensor; // Pointer to the level sensor
}pump_monitor_config_t;


typedef struct pump_control_unit_t  pump_control_unit_t;

pump_control_unit_t* pump_control_unit_create();
error_type_t pump_control_unit_init(pump_control_unit_t *manager);
error_type_t pump_control_unit_deinit(pump_control_unit_t *manager);
error_type_t pump_control_unit_destroy(pump_control_unit_t **manager);
error_type_t pump_control_unit_get_state(const pump_control_unit_t *manager, bool *is_initialized);
error_type_t pump_control_unit_add_tank_monitor(pump_control_unit_t *manager, tank_monitor_t *tank_monitor);
error_type_t pump_control_unit_remove_tank_monitor(pump_control_unit_t *manager, int tank_monitor_id);
error_type_t pump_control_unit_add_pump_monitor(pump_control_unit_t *manager, pump_monitor_t* pump_monitor);
error_type_t pump_control_unit_remove_pump_monitor(pump_control_unit_t *manager, int pump_monitor_id);
error_type_t pump_control_unit_add_relay(pump_control_unit_t *manager, relay_t *relay);
error_type_t pump_control_unit_remove_relay(pump_control_unit_t *manager, int relay_id);
error_type_t pump_control_unit_get_relay_by_id(pump_control_unit_t *manager, int relay_id, relay_t **relay);
error_type_t pump_control_add_relay_to_tank_monitor(pump_control_unit_t *manager, int tank_monitor_id, int relay_id);
error_type_t pump_control_unit_remove_relay_from_tank_monitor(pump_control_unit_t *manager, int tank_monitor_id, int relay_id);
error_type_t pump_control_unit_add_relay_to_pump_monitor(pump_control_unit_t *manager, int pump_monitor_id, int relay_id);
error_type_t pump_control_unit_remove_relay_from_pump_monitor(pump_control_unit_t *manager, int pump_monitor_id, int relay_id);
void pump_tank_event_loop(pump_control_unit_t *manager);
void callback_handler(void* context,int actuator_id, event_type_t state,int monitor_id);


#endif // __pump_control_unit_H__