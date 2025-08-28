#ifndef __SETUP_CONFIG_H__
#define __SETUP_CONFIG_H__
#include <common_headers.h>
#include <stdint.h>
#include "cJSON.h"

typedef enum {
    CURRENT_SENSOR_INTERFACE_I2C,
    CURRENT_SENSOR_INTERFACE_SPI,
    CURRENT_SENSOR_INTERFACE_PWM,
    CURRENT_SENSOR_INTERFACE_ANALOG_VOLTAGE
}current_sensor_interface_t;


error_type_t setup_config_tank (cJSON* tank_json);
error_type_t setup_config_pump(cJSON* pump_json);
error_type_t setup_config_pump_monitor(cJSON*pump_monitor_json);
error_type_t setup_config_tank_monitor(cJSON* tank_monitor_json);
error_type_t setup_config_relay_driver(cJSON* relay_json);
error_type_t setup_config_level_sensor(cJSON* level_sensor_json);
error_type_t setup_config_current_sensor(cJSON* current_sensor_json);


#endif