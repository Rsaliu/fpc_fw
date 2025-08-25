#ifndef __SETUP_CONFIG_H__
#define __SETUP_CONFIG_H__
#include <common_headers.h>
#include <stdint.h>
#include "cJSON.h"


error_type_t tank_Setup_config (cJSON* tank_json);
error_type_t pump_setup_config(cJSON* pump_json);
error_type_t pump_monitor_setup_config(cJSON*pump_monitor_json);
error_type_t tank_monitor_setup_config(cJSON* tank_monitor_json);
error_type_t relay_driver_setup_config(cJSON* relay_json);
error_type_t level_sensor_setup_config(cJSON* level_sensor_json);
error_type_t current_sensor_setup_confi(cJSON* current_sensor_json);

#endif