#ifndef __SETUP_CONFIG_H__
#define __SETUP_CONFIG_H__
#include <common_headers.h>
#include <stdint.h>
#include "cJSON.h"
#include <tank.h>
#include <validate_json.h>
#define MAX_SUBSCRIBERS_PER_MONITOR 5

typedef enum{
    ACS712,
    INVALID_CURRENT_SENSOR_MAKE
} current_sensor_make_config_t;

typedef enum{
    ADS1115_ONE,
    INTERNAL_ADC,
    INVALID_INTERFACE
} current_sensor_interface_type_t;

typedef enum{
    CURRENT_SENSOR_CONFIG_READ_MODE_BASIC,
    CURRENT_SENSOR_CONFIG_READ_CONTINUOUS,
    CURRENT_SENSOR_CONFIG_READ_OVERCURRENT_MONITOR,
    INVALID_READ_MODE
}current_sensor_read_mode_config_t;
typedef struct{
    current_sensor_interface_type_t interface;
    int channel;
}current_sensor_interface_config_t;
typedef struct{
    int current_sensor_id;
    current_sensor_interface_config_t interface;
    current_sensor_make_config_t current_sensor_make;
    int max_current;
    current_sensor_read_mode_config_t read_mode;
} current_sensor_setup_config_t;

typedef struct 
{
    int pump_id;
    char* pump_make;
    float pump_power_in_hp;
    float pump_current_rating;
    float pump_min_working_current;
}pump_setup_config_t;

// adding minimal pump monitor config for now, can be expanded later with more parameters as needed
typedef struct 
{
    int pump_monitor_id;
    int pump_id;
    int current_sensor_id;
}pump_monitor_setup_config_t;

typedef struct 
{
    int tank_monitor_id;
    int tank_id;
    int level_sensor_id;
}tank_monitor_setup_config_t;

typedef struct 
{
    int relay_id;
    int relay_pin_number;
}relay_setup_config_t;

typedef enum{
    SUBSCRIBER_TYPE_RELAY,
    INVALID_SUBSCRIBER_TYPE
}subscriber_type_t;

typedef enum{
    RELAY_RESPONSE_ONE,
    INVALID_RELAY_RESPONSE_TYPE
}relay_response_type_t;

typedef enum{
    GSM_RESPONSE_ONE,
    INVALID_GSM_RESPONSE_TYPE
}gsm_response_type_t;

typedef union{
    relay_response_type_t relay_response;
    gsm_response_type_t gsm_response;
}response_action_t;

typedef enum{
    TANK_MONITOR,
    PUMP_MONITOR,
    INVALID_MONITOR_TYPE
}monitor_type_t;

typedef struct 
{
    subscriber_type_t type;
    int id;
    response_action_t response;
}subscriber_t;

typedef struct 
{
    monitor_type_t monitor_type;
    int monitor_id;
    int num_subscribers;
    subscriber_t subscribers[MAX_SUBSCRIBERS_PER_MONITOR];
}susbscription_setup_config_t;

typedef enum{
    RS485,
    INVALID_LEVEL_SENSOR_INTERFACE
}level_sensor_interface_type_t;

typedef enum{
    GA1,
    INVALID_PROTOCOL
}level_sensor_protocol_type_t;

typedef struct{
    int level_sensor_id;
    level_sensor_interface_type_t interface;
    uint8_t sensor_addr;
    level_sensor_protocol_type_t protocol;
}level_sensor_setup_config_t;

typedef enum{
    TANK_SHAPE_CYLINDRICAL,
    TANK_SHAPE_RECTANGULAR,
    INVALID_TANK_SHAPE
}task_shape_type_t;
typedef struct 
{
    int tank_id;
    float tank_capacity;
    task_shape_type_t tank_shape;
    float tank_height_cm;
    int tank_full_level_mm;
    int tank_low_level_mm;
}tank_setup_config_t;


cJSON * deserialized_to_json(const char *json_str, size_t size);
cJSON * get_pump_control_json_array_from_root(const cJSON* root,size_t *array_size);
// remember to free the allocated current_sensor_configs after using it
error_type_t get_current_sensors_configs_from_pump_control_json(const cJSON* pump_control_unit_json,current_sensor_setup_config_t** current_sensor_configs, int* num_sensors);
error_type_t get_pumps_configs_from_pump_control_json(const cJSON* pump_control_unit_json, pump_setup_config_t** pump_configs, int* num_pumps);
error_type_t get_pump_monitors_configs_from_pump_control_json(const cJSON* pump_control_unit_json, pump_monitor_setup_config_t** pump_monitor_configs, int* num_pump_monitors);
error_type_t get_relays_configs_from_pump_control_json(const cJSON* pump_control_unit_json, relay_setup_config_t** relay_configs, int* num_relays);
error_type_t get_subscriptions_configs_from_pump_control_json(const cJSON* root, susbscription_setup_config_t** subscription_configs, int* num_subscriptions);
error_type_t get_tanks_configs_from_pump_control_json(const cJSON* pump_control_unit_json, tank_setup_config_t** tank_configs, int* num_tanks);
error_type_t get_level_sensors_configs_from_pump_control_json(const cJSON* pump_control_unit_json, level_sensor_setup_config_t** level_sensor_configs, int* num_level_sensors);
error_type_t get_tank_monitors_configs_from_pump_control_json(const cJSON* pump_control_unit_json, tank_monitor_setup_config_t** tank_monitor_configs, int* num_tank_monitors);

#endif