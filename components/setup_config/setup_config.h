#ifndef __SETUP_CONFIG_H__
#define __SETUP_CONFIG_H__
#include <common_headers.h>
#include <stdint.h>
#include "cJSON.h"
#include <tank.h>

typedef enum {
    CURRENT_SENSOR_INTERFACE_I2C,
    CURRENT_SENSOR_INTERFACE_SPI,
    CURRENT_SENSOR_INTERFACE_PWM,
    CURRENT_SENSOR_INTERFACE_ANALOG_VOLTAGE
}current_sensor_interface_t;

typedef struct 
{
    int unit_id;
    struct 
    {
        int tank_id;
        float tank_capacity;
        const char* tank_shape;
        float tank_height;
        int tank_full;
        int tank_low;
    }tank;

    struct 
    {
        int pump_id;
        char* pump_make;
        float pump_power_in_hp;
        float pump_current_rating;
    }pump;

    struct 
    {
        int tank_monitor_id;
        int level_sensor_id;
        int tank_id;
    }tank_monitor;

    struct 
    {
        int pump_monitor_id;
        int current_senor_id;
        int pump_id;
    }pump_monitor;

    struct 
    {
        int relay_id;
        int relay_pin_number;
    }relay;

    struct 
    {
        int level_sensor_id;
        const char* interface;
        uint8_t sensor_addr;
        const char* protocol;
        
    }level_sensor;

    struct 
    {
        int current_sensor_id;
        const char* interface;
        char* current_sensor_make;
        int max_current;
    }current_sensor;

}pump_control_unit_t;

pump_control_unit_t deserilalized_pump_control_unit(const char* json_str);
error_type_t setup_config_tank(pump_control_unit_t* pump_control_obj);
error_type_t setup_config_pump(pump_control_unit_t* pump_control_obj);
error_type_t setup_config_tank_monitor(pump_control_unit_t* pump_control_obj);
error_type_t setup_config_pump_monitor(pump_control_unit_t* pump_control_obj);
error_type_t setup_config_relay(pump_control_unit_t* pump_control_obj);
error_type_t setup_config_level_sensor(pump_control_unit_t* pump_control_obj);
error_type_t setup_config_current_sensor(pump_control_unit_t* pump_control_obj);


#endif