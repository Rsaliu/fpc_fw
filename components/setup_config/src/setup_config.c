#include <stdio.h>
#include <setup_config.h>
#include <string.h>
#include <tank.h>
#include <pump.h>
#include <tank_monitor.h>
#include <pump_monitor.h>
#include <relay_driver.h>
#include <current_sensor.h>
#include <level_sensor.h>
#include "esp_log.h"

static const char *TAG = "SETUP_CONFIG";

 static cJSON* get_first_pump_control_unit_array(cJSON* pump_control_unit_json) {
    cJSON* pump_control_units = cJSON_GetObjectItem(pump_control_unit_json, "pump_control_units");
    if (!pump_control_units || !cJSON_IsArray(pump_control_units)) {
        ESP_LOGE(TAG, "pump_control_units is missing or not an array");
        return NULL;
    }
    // get the first array of object in  pump control unit
    cJSON* pump_control_unit_array = cJSON_GetArrayItem(pump_control_units, 0);
    if (!pump_control_unit_array) {
        ESP_LOGE(TAG, "object array does not exit");
        return NULL;
    }

    return pump_control_unit_array;
}

error_type_t tank_Setup_config(cJSON *tank_json){
    cJSON* get_tank_arr = get_first_pump_control_unit_array(tank_json);
    if (!get_tank_arr)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    cJSON *tank = cJSON_GetObjectItem(get_tank_arr, "tank");
    if (!tank)
    {
        ESP_LOGE(TAG, "there is no tank object in json");
         return SYSTEM_INVALID_PARAMETER;

    }

    // read tank config from json
    int tank_id = cJSON_GetObjectItem(tank, "id")->valueint;
    float tank_capacity = cJSON_GetObjectItem(tank, "capcity_in_liters")->valuedouble;
    const char *shape_str = cJSON_GetObjectItem(tank, "shape")->valuestring;
    tank_shape_t tank_shape = string_to_tank_shape(shape_str);
    float tank_height = cJSON_GetObjectItem(tank, "height_in_cm")->valuedouble;
    int full_tank = cJSON_GetObjectItem(tank, "full_level_in_mm")->valueint;
    int low_tank = cJSON_GetObjectItem(tank, "low_level_in_mm")->valueint;

    tank_config_t tank_config = {
        .id = tank_id,
        .capacity_in_liters = tank_capacity,
        .shape = tank_shape,
        .height_in_cm = tank_height,
        .full_level_in_mm = full_tank,
        .low_level_in_mm = low_tank,

    };

    tank_t *tank_obj = tank_create(tank_config);
    if (!tank_obj)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    return SYSTEM_OK;
}

error_type_t pump_setup_config(cJSON * pump_json)
{
    cJSON* get_pump_arr   = get_first_pump_control_unit_array(pump_json);
    if (!get_pump_arr)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    cJSON *pump = cJSON_GetObjectItem(get_pump_arr, "pump");
    if (!pump)
    {
        ESP_LOGE(TAG, "there is no pump object in json file");
        return SYSTEM_INVALID_PARAMETER;

    }

    // Read pump config from json
    int pump_id = cJSON_GetObjectItem(pump, "id")->valueint;
    char *pump_make = cJSON_GetObjectItem(pump, "make")->valuestring;
    float pump_power_in_hp = cJSON_GetObjectItem(pump, "power_in_hp")->valuedouble;
    float current_rating = cJSON_GetObjectItem(pump, "current_rating")->valuedouble;

    pump_config_t pump_config = {
        .id = pump_id,
        .make = pump_make,
        .power_in_hp = pump_power_in_hp,
        .current_rating = current_rating,
    };

    pump_t *pump_obj = pump_create(pump_config);
    if (pump_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    return SYSTEM_OK;
}

error_type_t tank_monitor_setup_config(cJSON *tank_monitor_json)
{
    cJSON* get_tank_monitor_arr  = get_first_pump_control_unit_array(tank_monitor_json);
    if (!get_tank_monitor_arr )
    {
        return SYSTEM_NULL_PARAMETER;
    }

    cJSON *tank_monitor = cJSON_GetObjectItem(get_tank_monitor_arr, "tank_monitor");
    if (!tank_monitor)
    {
        ESP_LOGE(TAG, "there is no tank monitor object in json file");
        return SYSTEM_INVALID_PARAMETER;

    }

    // Read tank monitor config from json file
    int tank_monitor_id = cJSON_GetObjectItem(tank_monitor, "id")->valueint;
    int level_sensor_id = cJSON_GetObjectItem(tank_monitor, "level_sensor_id")->valueint;
    level_sensor_config_t level_sensor_config = {
        .id = level_sensor_id,
    };
    level_sensor_t *level_sensor = level_sensor_create(&level_sensor_config);

    int tank_id = cJSON_GetObjectItem(tank_monitor, "tank_id")->valueint;
    tank_config_t tank_config = {
        .id = tank_id,
    };
    tank_t *tank = tank_create(tank_config);

    tank_monitor_config_t tank_monitor_config = {
        .id = tank_monitor_id,
        .sensor = level_sensor,
        .tank = tank,
    };

    tank_monitor_t *tank_monitor_obj = tank_monitor_create(tank_monitor_config);
    if (!tank_monitor_obj)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    return SYSTEM_OK;
}

error_type_t pump_monitor_setup_config(cJSON *pump_monitor_json)
{
    cJSON* get_pump_monitor_arr   = get_first_pump_control_unit_array(pump_monitor_json);
    if (!get_pump_monitor_arr )
    {
        return SYSTEM_NULL_PARAMETER;
    }

    cJSON *pump_monitor = cJSON_GetObjectItem(get_pump_monitor_arr, "pump_monitor");
    if (!pump_monitor)
    {
        ESP_LOGE(TAG, "there is no pump monitor object in json file");
        return SYSTEM_INVALID_PARAMETER;

    }

    // Read pump control fom json
    int pump_monitor_id = cJSON_GetObjectItem(pump_monitor, "id")->valueint;
    int current_sensor_id = cJSON_GetObjectItem(pump_monitor, "current_sensor_id")->valueint;
    current_sensor_config_t current_sensor_config = {
        .id = current_sensor_id,
    };
    current_sensor_t *current_sensor_obj = current_sensor_create(&current_sensor_config);

    int pump_id = cJSON_GetObjectItem(pump_monitor, "pump_id")->valueint;
    pump_config_t pump_config = {
        .id = pump_id,
    };
    pump_t *pump_obj = pump_create(pump_config);

    pump_monitor_config_t pump_monitor_config = {
        .id = pump_monitor_id,
        .sensor = current_sensor_obj,
        .pump = pump_obj,
    };
    pump_monitor_t *pump_monitor_obj = pump_monitor_create(pump_monitor_config);
    if (pump_monitor_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    return SYSTEM_OK;
}

error_type_t relay_driver_setup_config(cJSON *relay_json)
{
    cJSON* get_relay_arr  = get_first_pump_control_unit_array(relay_json);
    if (!get_relay_arr )
    {
        return SYSTEM_NULL_PARAMETER;
    }

    cJSON *relay = cJSON_GetObjectItem(get_relay_arr, "relay");
    if (!relay)
    {
        ESP_LOGE(TAG, "there is no relay driver  object in json file");
        return SYSTEM_INVALID_PARAMETER;

    }

    // Read relay driver object from json
    int relay_id = cJSON_GetObjectItem(relay, "id")->valueint;
    uint8_t pin_number = cJSON_GetObjectItem(relay, "pin_number")->valueint;

    relay_config_t relay_config = {
        .id = relay_id,
        .relay_pin_number = pin_number,
    };

    relay_t *relay_obj = relay_create(&relay_config);
    if (relay_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    return SYSTEM_OK;
}

error_type_t level_sensor_setup_config(cJSON *level_sensor_json)
{
    cJSON* get_level_sensor_arr  = get_first_pump_control_unit_array(level_sensor_json);
    if (!get_level_sensor_arr )
    {
        return SYSTEM_NULL_PARAMETER;
    }

    cJSON *level_sensor = cJSON_GetObjectItem(get_level_sensor_arr, "level_sensor");
    if (!level_sensor)
    {
        ESP_LOGE(TAG, "there is no level_sensor object in json file");
        return SYSTEM_INVALID_PARAMETER;

    }

    // Read relay driver object from json
    int level_sensor_id = cJSON_GetObjectItem(level_sensor, "id")->valueint;
    const char *interface_str = cJSON_GetObjectItem(level_sensor, "interface")->valuestring;
    level_sensor_interface_t RS485 = string_to_level_sensor_interface(interface_str);
    uint8_t level_sensor_addr = cJSON_GetObjectItem(level_sensor, "sensor_addr")->valueint;
    const char *protocol_str = cJSON_GetObjectItem(level_sensor, "level_sensor_protocol")->valuestring;
    level_sensor_protocol_t GL_A01_PROTOCOL = string_to_level_sensor_protocol(protocol_str);

    level_sensor_config_t level_sensor_config = {
        .id = level_sensor_id,
        .interface = RS485,
        .sensor_addr = level_sensor_addr,
        .level_sensor_protocol = GL_A01_PROTOCOL,
    };

    level_sensor_t *level_sensor_obj = level_sensor_create(&level_sensor_config);
    if (level_sensor_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    return SYSTEM_OK;
}

error_type_t current_sensor_setup_confi(cJSON *current_sensor_json)
{
    cJSON* get_current_sensor_arr  = get_first_pump_control_unit_array(current_sensor_json);
    if (!get_current_sensor_arr)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    cJSON *current_sensor = cJSON_GetObjectItem(get_current_sensor_arr, "current_sensor");
    if (!current_sensor)
    {
        ESP_LOGE(TAG, "there is no current_sensor object in json file");
        return SYSTEM_INVALID_PARAMETER;
    }

    // Read relay driver object from json
    int current_sensor_id = cJSON_GetObjectItem(current_sensor, "id")->valueint;
    const char *str_interface = cJSON_GetObjectItem(current_sensor, "interface")->valuestring;
    current_sensor_interface_t current_sensor_interface = string_to_current_sensor_interface(str_interface);
    char *current_sensor_make = cJSON_GetObjectItem(current_sensor, "make")->valuestring;
    int maximum_current = cJSON_GetObjectItem(current_sensor, "max_current")->valueint;

    current_sensor_config_t current_sensor_config = {
        .id = current_sensor_id,
        .interface = current_sensor_interface,
        .make = current_sensor_make,
        .max_current = maximum_current,
    };

    current_sensor_t *current_sensor_obj = current_sensor_create(&current_sensor_config);
    if (current_sensor_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    return SYSTEM_OK;
}

