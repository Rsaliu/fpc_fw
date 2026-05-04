#include <stdio.h>
#include <setup_config.h>
#include <string.h>
#include <pump.h>
#include <tank_monitor.h>
#include <pump_monitor.h>
#include <relay_driver.h>
#include <level_sensor.h>
#include <current_sensor.h>
#include <protocol.h>
#include <rs485.h>
#include <ads1115.h>
#include "esp_log.h"

static const char *TAG = "SETUP_CONFIG";


cJSON * deserialized_to_json(const char *json_str, size_t size){
    cJSON *root = cJSON_ParseWithLength(json_str, size);
    if (!root)
    {
        ESP_LOGE(TAG, "failed to parse json string\n.");
        return NULL;
    }
    return root;
}

cJSON * get_pump_control_json_array_from_root(const cJSON* root,size_t *array_size){
    *array_size = 0;
    // get array of pump control units
    cJSON *units = cJSON_GetObjectItem(root, "pump_control_units");
    if (!units )
    {
        ESP_LOGE(TAG, "pump_control_units is NULL\n.");
        return NULL;
    }
    if(!cJSON_IsArray(units))
    {
        ESP_LOGE(TAG, "pump_control_units is not an array\n.");
        return NULL;
    }
    *array_size = cJSON_GetArraySize(units);
    return units;
}

static current_sensor_interface_type_t string_to_current_sensor_interface_type(const char* interface_str){
    if (strcmp("ADS1115_one", interface_str) == 0)
    {
        return ADS1115_ONE;
    }
    else if(strcmp("internal_adc", interface_str) == 0)
    {
        return INTERNAL_ADC;
    }
    else
    {
        ESP_LOGE(TAG, "unknown current sensor interface");
        return INVALID_INTERFACE;
    }
}
static current_sensor_read_mode_config_t string_to_current_sensor_read_mode(const char* read_mode_str){
    if (strcmp("basic", read_mode_str) == 0)
    {
        return CURRENT_SENSOR_CONFIG_READ_MODE_BASIC;
    }
    else if(strcmp("continuous", read_mode_str) == 0)
    {
        return CURRENT_SENSOR_CONFIG_READ_CONTINUOUS;
    }
    else if(strcmp("overcurrent_monitor", read_mode_str) == 0)
    {
        return CURRENT_SENSOR_CONFIG_READ_OVERCURRENT_MONITOR;
    }
    else
    {
        ESP_LOGE(TAG, "unknown current sensor read mode");
        return INVALID_READ_MODE;
    }
}

static current_sensor_make_config_t string_to_current_sensor_make(const char* make_str){
    if (strcmp("ACS712", make_str) == 0)
    {
        return ACS712;
    }
    else
    {
        ESP_LOGE(TAG, "unknown current sensor make");
        return INVALID_CURRENT_SENSOR_MAKE;
    }
}

error_type_t get_current_sensors_configs_from_pump_control_json(const cJSON* pump_control_unit_json,current_sensor_setup_config_t** current_sensor_configs, int* num_sensors){
    if (pump_control_unit_json == NULL || current_sensor_configs == NULL || num_sensors == NULL)
    {
        ESP_LOGE(TAG, "invalid parameter passed to current sensor parser\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    *num_sensors = 0;
    *current_sensor_configs = NULL;
    cJSON *current_sensor_json = cJSON_GetObjectItem(pump_control_unit_json, "current_sensors");
    // current sensor is an array of objects
    if (!current_sensor_json || !cJSON_IsArray(current_sensor_json))
    {
        ESP_LOGE(TAG, "current_sensors is empty/null or invalid array\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    int sensor_array_size = cJSON_GetArraySize(current_sensor_json);
    *current_sensor_configs = (current_sensor_setup_config_t*)malloc(sizeof(current_sensor_setup_config_t)*sensor_array_size);
    if (*current_sensor_configs == NULL)
    {
        ESP_LOGE(TAG, "failed to allocate current sensor config array\n.");
        return SYSTEM_FAILED;
    }
    /* sample schema for each current sensor object in the array
                    {
                "Id": 1,
                "interface":{
                    "type": "ADS1115_one",
                    "channel": 1
                },
                "make": "ACS712",
                "max_current": 20,
                "read_mode" : "basic"
            }
    */
    int index = 0;
    for (int i = 0; i < sensor_array_size; i++){
        cJSON *current_sensor_obj = cJSON_GetArrayItem(current_sensor_json, i);
        if (!current_sensor_obj || !cJSON_IsObject(current_sensor_obj))
        {
            ESP_LOGE(TAG, "current sensor object is empty/null or invalid object\n.");
            continue;
        }
        cJSON *id = cJSON_GetObjectItem(current_sensor_obj, "id");
        if (!id || !cJSON_IsNumber(id))
        {
            ESP_LOGE(TAG, "current sensor id is empty/null or invalid int\n.");
            continue;
        }
        (*current_sensor_configs)[index].current_sensor_id = id->valueint;

        cJSON *interface = cJSON_GetObjectItem(current_sensor_obj, "interface");
        if (!interface || !cJSON_IsObject(interface))
        {
            ESP_LOGE(TAG, "current sensor interface is empty/null or invalid object\n.");
            continue;
        }
        cJSON *interface_type = cJSON_GetObjectItem(interface, "type");
        if (!interface_type || !cJSON_IsString(interface_type))
        {
            ESP_LOGE(TAG, "current sensor interface type is empty/null or invalid string\n.");
            continue;
        }
        (*current_sensor_configs)[index].interface.interface = string_to_current_sensor_interface_type(interface_type->valuestring);

        cJSON *channel = cJSON_GetObjectItem(interface, "channel");
        if (!channel || !cJSON_IsNumber(channel))
        {
            ESP_LOGE(TAG, "current sensor interface channel is empty/null or invalid int\n.");
            continue;
        }
        (*current_sensor_configs)[index].interface.channel = channel->valueint;

        cJSON *make = cJSON_GetObjectItem(current_sensor_obj, "make");
        if (!make || !cJSON_IsString(make))
        {
            ESP_LOGE(TAG, "current sensor make is empty/null or invalid string\n.");
            continue;
        }
        (*current_sensor_configs)[index].current_sensor_make = string_to_current_sensor_make(make->valuestring);

         cJSON *max_current = cJSON_GetObjectItem(current_sensor_obj, "max_current");
        if (!max_current || !cJSON_IsNumber(max_current))
        {
            ESP_LOGE(TAG, "current sensor max current is empty/null or invalid number\n.");
            continue;
        }
        (*current_sensor_configs)[index].max_current = max_current->valueint;

        cJSON *read_mode = cJSON_GetObjectItem(current_sensor_obj, "read_mode");
        if (!read_mode || !cJSON_IsString(read_mode))
        {
            ESP_LOGE(TAG, "current sensor read mode is empty/null or invalid string\n.");
            continue;
        }
        (*current_sensor_configs)[index].read_mode = string_to_current_sensor_read_mode(read_mode->valuestring);
        *num_sensors = ++index;
    }
    return SYSTEM_OK;
}

error_type_t get_pumps_configs_from_pump_control_json(const cJSON* pump_control_unit_json, pump_setup_config_t** pump_configs, int* num_pumps){
    *num_pumps = 0;
    cJSON *pump_json = cJSON_GetObjectItem(pump_control_unit_json, "pumps");
    // pump is an array of objects
    if (!pump_json || !cJSON_IsArray(pump_json))
    {
        ESP_LOGE(TAG, "pump is empty/null or invalid array\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    int pump_array_size = cJSON_GetArraySize(pump_json);
    *pump_configs = (pump_setup_config_t*)malloc(sizeof(pump_setup_config_t)*pump_array_size);
    /* sample schema for each pump object in the array
            {
                "id": 1,
                "make": "Grundfos",
                "power_in_hp": 0.5,
                "current_rating": 2.5,
                "min_working_current": 0.5
            }
    */
   int index = 0;
   for (int i = 0; i < pump_array_size; i++){
        cJSON *pump_obj = cJSON_GetArrayItem(pump_json, i);
        if (!pump_obj || !cJSON_IsObject(pump_obj))
        {
            ESP_LOGE(TAG, "pump object is empty/null or invalid object\n.");
            continue;
        }
        cJSON *id = cJSON_GetObjectItem(pump_obj, "id");
        if (!id || !cJSON_IsNumber(id))
        {
            ESP_LOGE(TAG, "pump id is empty/null or invalid int\n.");
            continue;
        }
        pump_configs[index]->pump_id = id->valueint;

        cJSON *make = cJSON_GetObjectItem(pump_obj, "make");
        if (!make || !cJSON_IsString(make))
        {
            ESP_LOGE(TAG, "pump make is empty/null or invalid string\n.");
            continue;
        }
        pump_configs[index]->pump_make = make->valuestring;

        cJSON *power_in_hp = cJSON_GetObjectItem(pump_obj, "power_in_hp");
        if (!power_in_hp || !cJSON_IsNumber(power_in_hp))
        {
            ESP_LOGE(TAG, "pump power in hp is empty/null or invalid number\n.");
            continue;
        }
        pump_configs[index]->pump_power_in_hp = power_in_hp->valuedouble;

         cJSON *current_rating = cJSON_GetObjectItem(pump_obj, "current_rating");
        if (!current_rating || !cJSON_IsNumber(current_rating))
        {
            ESP_LOGE(TAG, "pump current rating is empty/null or invalid number\n.");
            continue;
        }
        pump_configs[index]->pump_current_rating = current_rating->valuedouble;
        cJSON *min_working_current = cJSON_GetObjectItem(pump_obj, "min_working_current");
        if (!min_working_current || !cJSON_IsNumber(min_working_current))
        {
            ESP_LOGE(TAG, "pump min working current is empty/null or invalid number\n.");
            continue;
        }
        pump_configs[index]->pump_min_working_current = min_working_current->valuedouble;
        *num_pumps = ++index;
    }
    return SYSTEM_OK;
}

error_type_t get_pump_monitors_configs_from_pump_control_json(const cJSON* pump_control_unit_json, pump_monitor_setup_config_t** pump_monitor_configs, int* num_pump_monitors){
    *num_pump_monitors = 0;
    cJSON *pump_monitor_json = cJSON_GetObjectItem(pump_control_unit_json, "pump_monitors");
    // pump monitor is an array of objects
    if (!pump_monitor_json || !cJSON_IsArray(pump_monitor_json))
    {
        ESP_LOGE(TAG, "pump monitor is empty/null or invalid array\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    int pump_monitor_array_size = cJSON_GetArraySize(pump_monitor_json);
    *pump_monitor_configs = (pump_monitor_setup_config_t*)malloc(sizeof(pump_monitor_setup_config_t)*pump_monitor_array_size);
    /* sample schema for each pump monitor object in the array
            {
                "id": 1,
                "pump_id": 1,
                "current_sensor_id": 1
            }
    */
   int index = 0;
   for (int i = 0; i < pump_monitor_array_size; i++){
        cJSON *pump_monitor_obj = cJSON_GetArrayItem(pump_monitor_json, i);
        if (!pump_monitor_obj || !cJSON_IsObject(pump_monitor_obj))
        {
            ESP_LOGE(TAG, "pump monitor object is empty/null or invalid object\n.");
            continue;
        }
        cJSON *id = cJSON_GetObjectItem(pump_monitor_obj, "id");
        if (!id || !cJSON_IsNumber(id))
        {
            ESP_LOGE(TAG, "pump monitor id is empty/null or invalid int\n.");
            continue;
        }
        pump_monitor_configs[index]->pump_monitor_id = id->valueint;

        cJSON *pump_id = cJSON_GetObjectItem(pump_monitor_obj, "pump_id");
        if (!pump_id || !cJSON_IsNumber(pump_id))
        {
            ESP_LOGE(TAG, "pump monitor pump id is empty/null or invalid int\n.");
            continue;
        }
        pump_monitor_configs[index]->pump_id = pump_id->valueint;

        cJSON *current_sensor_id = cJSON_GetObjectItem(pump_monitor_obj, "current_sensor_id");
        if (!current_sensor_id || !cJSON_IsNumber(current_sensor_id))
        {
            ESP_LOGE(TAG, "pump monitor current sensor id is empty/null or invalid int\n.");
            continue;
        }
        pump_monitor_configs[index]->current_sensor_id = current_sensor_id->valueint;
        *num_pump_monitors = ++index;
    }
    return SYSTEM_OK;
}

error_type_t get_relays_configs_from_pump_control_json(const cJSON* pump_control_unit_json, relay_setup_config_t** relay_configs, int* num_relays){
    *num_relays = 0;
    cJSON *relay_json = cJSON_GetObjectItem(pump_control_unit_json, "relays");
    // relay is an array of objects
    if (!relay_json || !cJSON_IsArray(relay_json))
    {
        ESP_LOGE(TAG, "relay is empty/null or invalid array\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    int relay_array_size = cJSON_GetArraySize(relay_json);
    *relay_configs = (relay_setup_config_t*)malloc(sizeof(relay_setup_config_t)*relay_array_size);
    /* sample schema for each relay object in the array
            {
                "id": 1,
                "pin_number": 5
            }
    */
   int index = 0;
   for (int i = 0; i < relay_array_size; i++){
        cJSON *relay_obj = cJSON_GetArrayItem(relay_json, i);
        if (!relay_obj || !cJSON_IsObject(relay_obj))
        {
            ESP_LOGE(TAG, "relay object is empty/null or invalid object\n.");
            continue;
        }
        cJSON *id = cJSON_GetObjectItem(relay_obj, "id");
        if (!id || !cJSON_IsNumber(id))
        {
            ESP_LOGE(TAG, "relay id is empty/null or invalid int\n.");
            continue;
        }
        relay_configs[index]->relay_id = id->valueint;

        cJSON *pin_number = cJSON_GetObjectItem(relay_obj, "pin_number");
        if (!pin_number || !cJSON_IsNumber(pin_number))
        {
            ESP_LOGE(TAG, "relay pin number is empty/null or invalid int\n.");
            continue;
        }
        relay_configs[index]->relay_pin_number = pin_number->valueint;
        *num_relays = ++index;
    }
    return SYSTEM_OK;
}

static monitor_type_t string_to_monitor_type(const char* monitor_type_str){
    if (strcmp("TANK_MONITOR", monitor_type_str) == 0)
    {
        return TANK_MONITOR;
    }
    else if(strcmp("PUMP_MONITOR", monitor_type_str) == 0)
    {
        return PUMP_MONITOR;
    }
    else
    {
        ESP_LOGE(TAG, "unknown monitor type");
        return INVALID_MONITOR_TYPE;
    }
}

static error_type_t string_to_response_type(const char* response_type_str,response_action_t* response_action){
    if (strcmp("RELAY_RESPONSE_ONE", response_type_str) == 0)
    {
        (*response_action).relay_response = RELAY_RESPONSE_ONE;
        return SYSTEM_OK;
    }
    else
    {
        ESP_LOGE(TAG, "unknown response type");
                return SYSTEM_INVALID_PARAMETER;
    }
}

static subscriber_type_t string_to_subscriber_type(const char* subscriber_type_str){
    if (strcmp("RELAY", subscriber_type_str) == 0)
    {
        return SUBSCRIBER_TYPE_RELAY;
    }
    else
    {
        ESP_LOGE(TAG, "unknown subscriber type");
        return INVALID_SUBSCRIBER_TYPE;
    }
}

error_type_t get_subscriptions_configs_from_pump_control_json(const cJSON* pump_control_unit_json, susbscription_setup_config_t** subscription_configs, int* num_subscriptions){
    *num_subscriptions = 0;
    cJSON *subscriptions_json = cJSON_GetObjectItem(pump_control_unit_json, "subscriptions");
    // subscriptions is an array of objects
    if (!subscriptions_json || !cJSON_IsArray(subscriptions_json))
    {
        ESP_LOGE(TAG, "subscriptions is empty/null or invalid array\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    int subscriptions_array_size = cJSON_GetArraySize(subscriptions_json);
    *subscription_configs = (susbscription_setup_config_t*)malloc(sizeof(susbscription_setup_config_t)*subscriptions_array_size);
    /* sample schema for each subscription object in the array
    {
      "monitor_type": "TANK_MONITOR", // or PUMP_MONITOR depending on the monitor type
      "monitor_id": 1, 
      "subscribers": [
        {
            "type": "RELAY",
            "id": 1,
            "response type": "RELAY_RESPONSE_ONE"
        }
      ]
    }
    */
   int index = 0;
   for (int i = 0; i < subscriptions_array_size; i++){
        cJSON *subscription_obj = cJSON_GetArrayItem(subscriptions_json, i);
        if (!subscription_obj || !cJSON_IsObject(subscription_obj))
        {
            ESP_LOGE(TAG, "subscription object is empty/null or invalid object\n.");
            continue;
        }
        cJSON *monitor_type = cJSON_GetObjectItem(subscription_obj, "monitor_type");
        if (!monitor_type || !cJSON_IsString(monitor_type))
        {
            ESP_LOGE(TAG, "subscription monitor type is empty/null or invalid string\n.");
            continue;
        }
        monitor_type_t type = string_to_monitor_type(monitor_type->valuestring);
        if (type == INVALID_MONITOR_TYPE)
        {
            ESP_LOGE(TAG, "subscription monitor type is invalid\n.");
            continue;
        }
        (*subscription_configs)[index].monitor_type = type;

        cJSON *monitor_id = cJSON_GetObjectItem(subscription_obj, "monitor_id");
        if (!monitor_id || !cJSON_IsNumber(monitor_id))
        {
            ESP_LOGE(TAG, "subscription monitor id is empty/null or invalid int\n.");
            continue;
        }
        (*subscription_configs)[index].monitor_id = monitor_id->valueint;

        cJSON *subscribers = cJSON_GetObjectItem(subscription_obj, "subscribers");
        if (!subscribers || !cJSON_IsArray(subscribers))
        {
            ESP_LOGE(TAG, "subscription subscribers is empty/null or invalid array\n.");
            continue;
        }
        int subscribers_array_size = cJSON_GetArraySize(subscribers);
        if(subscribers_array_size > MAX_SUBSCRIBERS_PER_MONITOR)
        {
            ESP_LOGE(TAG, "number of subscribers exceeded the maximum allowed\n.");
            continue;
        }
        int subscriber_index = 0;
        for (int j = 0; j < subscribers_array_size; j++){
            cJSON *subscriber_obj = cJSON_GetArrayItem(subscribers, j);
            if (!subscriber_obj || !cJSON_IsObject(subscriber_obj))
            {
                ESP_LOGE(TAG, "subscriber object is empty/null or invalid object\n.");
                continue;
            }
            cJSON *type = cJSON_GetObjectItem(subscriber_obj, "type");
            if (!type || !cJSON_IsString(type))
            {
                ESP_LOGE(TAG, "subscriber type is empty/null or invalid string\n.");
                continue;
            }
            subscriber_type_t subscriber_type = string_to_subscriber_type(type->valuestring);
            if (subscriber_type == INVALID_SUBSCRIBER_TYPE)
            {
                ESP_LOGE(TAG, "subscriber type is invalid\n.");
                continue;
            }
            (*subscription_configs)[index].subscribers[j].type = subscriber_type;

             cJSON *id = cJSON_GetObjectItem(subscriber_obj, "id");
            if (!id || !cJSON_IsNumber(id))
            {
                ESP_LOGE(TAG, "subscriber id is empty/null or invalid int\n.");
                continue;
            }
            (*subscription_configs)[index].subscribers[j].id = id->valueint;

            cJSON *response_type = cJSON_GetObjectItem(subscriber_obj, "response_type");
            if (!response_type || !cJSON_IsString(response_type))
            {
                ESP_LOGE(TAG, "subscriber response type is empty/null or invalid string\n.");
                continue;
            }
            response_action_t response_action;
            error_type_t err = string_to_response_type(response_type->valuestring,&response_action);
            if (err != SYSTEM_OK)
            {
                ESP_LOGE(TAG, "subscriber response type is invalid\n.");
                continue;
            }
            (*subscription_configs)[index].subscribers[j].response = response_action;  
            subscriber_index++;  
        }
        (*subscription_configs)[index].num_subscribers = subscriber_index;
        *num_subscriptions = ++index;
    }
    return SYSTEM_OK;
}

static task_shape_type_t setup_string_to_tank_shape(const char* shape_str){
    if (strcmp("CYLINDERICAL", shape_str) == 0)
    {
        return TANK_SHAPE_CYLINDRICAL;
    }
    else if(strcmp("RECTANGULAR", shape_str) == 0)
    {
        return TANK_SHAPE_RECTANGULAR;
    }
    else
    {
        ESP_LOGE(TAG, "unknown tank shape");
        return INVALID_TANK_SHAPE;
    }
}

error_type_t get_tanks_configs_from_pump_control_json(const cJSON* pump_control_unit_json, tank_setup_config_t** tank_configs, int* num_tanks){
    *num_tanks = 0;
    cJSON *tank_json = cJSON_GetObjectItem(pump_control_unit_json, "tanks");
    // tank is an array of objects
    if (!tank_json || !cJSON_IsArray(tank_json))
    {
        ESP_LOGE(TAG, "tank is empty/null or invalid array\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    int tank_array_size = cJSON_GetArraySize(tank_json);
    *tank_configs = (tank_setup_config_t*)malloc(sizeof(tank_setup_config_t)*tank_array_size);
    /* sample schema for each tank object in the array
            {
                "id": 1,
                "capacity_in_liters": 1000,
                "shape": "cylindrical",
                "height_in_cm": 150,
                "full_level_in_mm": 1400,
                "low_level_in_mm": 200
            }
    */
   int index = 0;
   for (int i = 0; i < tank_array_size; i++){
        cJSON *tank_obj = cJSON_GetArrayItem(tank_json, i);
        if (!tank_obj || !cJSON_IsObject(tank_obj))
        {
            ESP_LOGE(TAG, "tank object is empty/null or invalid object\n.");
            continue;
        }
        cJSON *id = cJSON_GetObjectItem(tank_obj, "id");
        if (!id || !cJSON_IsNumber(id))
        {
            ESP_LOGE(TAG, "tank id is empty/null or invalid int\n.");
            continue;
        }
        tank_configs[index]->tank_id = id->valueint;

        cJSON *capacity_in_liters = cJSON_GetObjectItem(tank_obj, "capacity_litres");
        if (!capacity_in_liters || !cJSON_IsNumber(capacity_in_liters))
        {
            ESP_LOGE(TAG, "tank capacity in liters is empty/null or invalid number\n.");
            continue;
        }
        tank_configs[index]->tank_capacity = capacity_in_liters->valuedouble;

        cJSON *shape = cJSON_GetObjectItem(tank_obj, "shape");
        if (!shape || !cJSON_IsString(shape))
        {
            ESP_LOGE(TAG, "tank shape is empty/null or invalid string\n.");
            continue;
        }
        tank_configs[index]->tank_shape = setup_string_to_tank_shape(shape->valuestring);
        if(tank_configs[index]->tank_shape == INVALID_TANK_SHAPE)
        {
            ESP_LOGE(TAG, "tank shape is invalid\n.");
            continue;
        }
        cJSON *height_in_cm = cJSON_GetObjectItem(tank_obj, "height_cm");
        if (!height_in_cm || !cJSON_IsNumber(height_in_cm)){
            ESP_LOGE(TAG, "tank height is empty/null or invalid string\n.");
            continue;
        }
        tank_configs[index]->tank_height_cm = height_in_cm->valuedouble;
        cJSON *full_level_in_mm = cJSON_GetObjectItem(tank_obj, "full_level_mm");
        if (!full_level_in_mm || !cJSON_IsNumber(full_level_in_mm)){
            ESP_LOGE(TAG, "tank full level is empty/null or invalid string\n.");
            continue;
        }
        tank_configs[index]->tank_full_level_mm = full_level_in_mm->valueint;
        cJSON *low_level_in_mm = cJSON_GetObjectItem(tank_obj, "low_level_mm");
        if (!low_level_in_mm || !cJSON_IsNumber(low_level_in_mm)){
            ESP_LOGE(TAG, "tank low level is empty/null or invalid string\n.");
            continue;
        }
        tank_configs[index]->tank_low_level_mm = low_level_in_mm->valueint;
        *num_tanks = ++index;
    }
    return SYSTEM_OK;
}

static level_sensor_interface_type_t string_to_level_sensor_interface_type(const char* interface_str){
    if (strcmp("RS485", interface_str) == 0)
    {
        return RS485;
    }
    else
    {
        ESP_LOGE(TAG, "unknown level sensor interface");
        return INVALID_INTERFACE;
    }
}

static level_sensor_protocol_type_t string_to_level_sensor_protocol_type(const char* protocol_str){
    if (strcmp("GA1", protocol_str) == 0)
    {
        return GA1;
    }
    else
    {
        ESP_LOGE(TAG, "unknown level sensor protocol");
        return INVALID_PROTOCOL;
    }
}

error_type_t get_level_sensors_configs_from_pump_control_json(const cJSON* pump_control_unit_json, level_sensor_setup_config_t** level_sensor_configs, int* num_level_sensors){
    *num_level_sensors = 0;
    cJSON *level_sensor_json = cJSON_GetObjectItem(pump_control_unit_json, "level_sensors");
    // level sensor is an array of objects
    if (!level_sensor_json || !cJSON_IsArray(level_sensor_json))
    {
        ESP_LOGE(TAG, "level sensor is empty/null or invalid array\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    int level_sensor_array_size = cJSON_GetArraySize(level_sensor_json);
    *level_sensor_configs = (level_sensor_setup_config_t*)malloc(sizeof(level_sensor_setup_config_t)*level_sensor_array_size);
    /* sample schema for each level sensor object in the array
        {
        "id": 1,
        "interface": "RS485",
        "address": 1,
        "protocol": "GA1"
        }
    */
    int index = 0;
    for (int i = 0; i < level_sensor_array_size; i++){
        cJSON *level_sensor_obj = cJSON_GetArrayItem(level_sensor_json, i);
        if (!level_sensor_obj || !cJSON_IsObject(level_sensor_obj))
        {
            ESP_LOGE(TAG, "level sensor object is empty/null or invalid object\n.");
            continue;
        }
        cJSON *id = cJSON_GetObjectItem(level_sensor_obj, "id");
        if (!id || !cJSON_IsNumber(id))
        {
            ESP_LOGE(TAG, "level sensor id is empty/null or invalid int\n.");
            continue;
        }
        (*level_sensor_configs)[index].level_sensor_id = id->valueint;

        cJSON *interface = cJSON_GetObjectItem(level_sensor_obj, "interface");
        if (!interface || !cJSON_IsString(interface))
        {
            ESP_LOGE(TAG, "level sensor interface is empty/null or invalid string\n.");
            continue;
        }
        (*level_sensor_configs)[index].interface = string_to_level_sensor_interface_type(interface->valuestring);
        if((*level_sensor_configs)[index].interface == INVALID_LEVEL_SENSOR_INTERFACE)
        {
            ESP_LOGE(TAG, "level sensor interface is invalid\n.");
            continue;
        }
        cJSON *address = cJSON_GetObjectItem(level_sensor_obj, "address");
        if (!address || !cJSON_IsNumber(address))
        {
            ESP_LOGE(TAG, "level sensor address is empty/null or invalid int\n.");
            continue;
        }
        (*level_sensor_configs)[index].sensor_addr = address->valueint;

         cJSON *protocol = cJSON_GetObjectItem(level_sensor_obj, "protocol");
        if (!protocol || !cJSON_IsString(protocol))
        {
            ESP_LOGE(TAG, "level sensor protocol is empty/null or invalid string\n.");
            continue;
        }
        (*level_sensor_configs)[index].protocol = string_to_level_sensor_protocol_type(protocol->valuestring);
        if((*level_sensor_configs)[index].protocol == INVALID_PROTOCOL)
        {
            ESP_LOGE(TAG, "level sensor protocol is invalid\n.");
            continue;
        }
        *num_level_sensors = ++index;
    }    
    return SYSTEM_OK;
}

error_type_t get_tank_monitors_configs_from_pump_control_json(const cJSON* pump_control_unit_json, tank_monitor_setup_config_t** tank_monitor_configs, int* num_tank_monitors){
    *num_tank_monitors = 0;
    cJSON *tank_monitor_json = cJSON_GetObjectItem(pump_control_unit_json, "tank_monitors");
    // tank monitor is an array of objects
    if (!tank_monitor_json || !cJSON_IsArray(tank_monitor_json))
    {
        ESP_LOGE(TAG, "tank monitor is empty/null or invalid array\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    int tank_monitor_array_size = cJSON_GetArraySize(tank_monitor_json);
    *tank_monitor_configs = (tank_monitor_setup_config_t*)malloc(sizeof(tank_monitor_setup_config_t)*tank_monitor_array_size);
    /* sample schema for each tank monitor object in the array
            {
                "id": 1,
                "tank_id": 1,
                "level_sensor_id": 1
            }
    */
   int index = 0;
   for (int i = 0; i < tank_monitor_array_size; i++){
        cJSON *tank_monitor_obj = cJSON_GetArrayItem(tank_monitor_json, i);
        if (!tank_monitor_obj || !cJSON_IsObject(tank_monitor_obj))
        {
            ESP_LOGE(TAG, "tank monitor object is empty/null or invalid object\n.");
            continue;
        }
        cJSON *id = cJSON_GetObjectItem(tank_monitor_obj, "id");
        if (!id || !cJSON_IsNumber(id))
        {
            ESP_LOGE(TAG, "tank monitor id is empty/null or invalid int\n.");
            continue;
        }
        (*tank_monitor_configs)[index].tank_monitor_id = id->valueint;

        cJSON *tank_id = cJSON_GetObjectItem(tank_monitor_obj, "tank_id");
        if (!tank_id || !cJSON_IsNumber(tank_id))
        {
            ESP_LOGE(TAG, "tank monitor tank id is empty/null or invalid int\n.");
            continue;
        }
        (*tank_monitor_configs)[index].tank_id = tank_id->valueint;

        cJSON *level_sensor_id = cJSON_GetObjectItem(tank_monitor_obj, "level_sensor_id");
        if (!level_sensor_id || !cJSON_IsNumber(level_sensor_id))
        {
            ESP_LOGE(TAG, "tank monitor level sensor id is empty/null or invalid int\n.");
            continue;
        }
        (*tank_monitor_configs)[index].level_sensor_id = level_sensor_id->valueint;
        *num_tank_monitors = ++index;
    }
    return SYSTEM_OK;
}
