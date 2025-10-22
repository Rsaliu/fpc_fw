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
rs485_t *rs485Obj;
ads1115_t*adc1115_obj;

error_type_t deserilalized_pump_control_unit(pump_control_unit_t *unit, const char *json_str)
{
    cJSON *root = cJSON_Parse(json_str);
    if (!root)
    {
        ESP_LOGE(TAG, "failed to parse json string\n.");
        return SYSTEM_INVALID_PARAMETER;
    }

    //  deserilized unit id
    cJSON *id = cJSON_GetObjectItem(root, "id");
    if (!id || !cJSON_IsNumber(id))
    {
        ESP_LOGE(TAG, "unit id is empty/null or invalid int\n.");
        cJSON_Delete(root);
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->unit_id = id->valueint;
    // deserilized tank
    cJSON *tank = cJSON_GetObjectItem(root, "tank");
    if (!tank || !cJSON_IsObject(tank))
    {
        ESP_LOGE(TAG, "invalid tank objects");
        return SYSTEM_INVALID_PARAMETER;
    }
    cJSON *t_id = cJSON_GetObjectItem(tank, "id");
    if (!t_id || !cJSON_IsNumber(t_id))
    {
        ESP_LOGE(TAG, "tank id is empty/null or invalid int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->tank.tank_id = t_id->valueint;
    cJSON *t_capacity = cJSON_GetObjectItem(tank, "capacity_in_liters");
    if (!t_capacity || !cJSON_IsNumber(t_capacity))
    {
        ESP_LOGE(TAG, "tank capacity is empty/null or invalid int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->tank.tank_capacity = t_capacity->valuedouble;
    cJSON *t_shape = cJSON_GetObjectItem(tank, "shape");
    if (!t_shape || !cJSON_IsString(t_shape))
    {
        ESP_LOGE(TAG, "tank shape retured an empty/null string\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->tank.tank_shape = t_shape->valuestring;
    cJSON *t_height = cJSON_GetObjectItem(tank, "height_in_cm");
    if (!t_height || !cJSON_IsNumber(t_height))
    {
        ESP_LOGE(TAG, "tank height retured abd empty/null float.\n");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->tank.tank_height = t_height->valuedouble;
    cJSON *t_full = cJSON_GetObjectItem(tank, "full_level_in_mm");
    if (!t_full || !cJSON_IsNumber(t_full))
    {
        ESP_LOGE(TAG, "tank full level retured an empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->tank.tank_full = t_full->valueint;
    cJSON *t_low = cJSON_GetObjectItem(tank, "low_level_in_mm");
    if (!t_low || !cJSON_IsNumber(t_low))
    {
        ESP_LOGE(TAG, "tank low level retured an empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->tank.tank_low = t_low->valueint;

    // deserilized pump
    cJSON *pump = cJSON_GetObjectItem(root, "pump");
    if (!pump || !cJSON_IsObject(pump))
    {
        ESP_LOGE(TAG, "invalid pump object\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    cJSON *p_id = cJSON_GetObjectItem(pump, "id");
    if (!p_id || !cJSON_IsNumber(p_id))
    {
        ESP_LOGE(TAG, "pump id returned an empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->pump.pump_id = p_id->valueint;
    cJSON *p_make = cJSON_GetObjectItem(pump, "make");
    if (!p_make || !cJSON_IsString(p_make))
    {
        ESP_LOGE(TAG, "pump make retured an empty/null string");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->pump.pump_make = p_make->valuestring;
    cJSON *p_power = cJSON_GetObjectItem(pump, "power_in_hp");
    if (!p_power || !cJSON_IsNumber(p_power))
    {
        ESP_LOGE(TAG, "pump power in hp retured an empty/null float\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->pump.pump_power_in_hp = p_power->valuedouble;
    cJSON *p_current_rating = cJSON_GetObjectItem(pump, "current_rating");
    if (!p_current_rating || !cJSON_IsNumber(p_current_rating))
    {
        ESP_LOGE(TAG, "pump current rating retured an empty/null float\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->pump.pump_current_rating = p_current_rating->valuedouble;

    // deseriliized tank_monitor
    cJSON *tank_monitor = cJSON_GetObjectItem(root, "tank_monitor");
    if (!tank_monitor || !cJSON_IsObject(tank_monitor))
    {
        ESP_LOGE(TAG, "invalid tank_monitor object\n");
        return SYSTEM_INVALID_PARAMETER;
    }
    cJSON *tank_monitor_id = cJSON_GetObjectItem(tank_monitor, "id");
    if (!tank_monitor_id || !cJSON_IsNumber(tank_monitor_id))
    {
        ESP_LOGE(TAG, "tank_monitor_id returned empty/null int\n");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->tank_monitor.tank_id = tank_monitor_id->valueint;
    cJSON *level_sensor_id = cJSON_GetObjectItem(tank_monitor, "level_sensor_id");
    if (!level_sensor_id || !cJSON_IsNumber(level_sensor_id))
    {
        ESP_LOGE(TAG, "tank_monitor level_sensor_id retured empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->tank_monitor.level_sensor_id = level_sensor_id->valueint;
    cJSON *tank_id = cJSON_GetObjectItem(tank_monitor, "tank_id");
    if (!tank_id || !cJSON_IsNumber(tank_id))
    {
        ESP_LOGE(TAG, "tank_monitor tank_id retured empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->tank_monitor.tank_id = tank_id->valueint;

    // deserilized pump_monitor
    cJSON *pump_monitor = cJSON_GetObjectItem(root, "pump_monitor");
    if (!pump_monitor || !cJSON_IsObject(pump_monitor))
    {
        ESP_LOGE(TAG, "invalid pump monitor object\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    cJSON *pump_monitor_id = cJSON_GetObjectItem(pump_monitor, "id");
    if (!pump_monitor_id || !cJSON_IsNumber(pump_monitor_id))
    {
        ESP_LOGE(TAG, "pump monitor id retured an empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->pump_monitor.pump_monitor_id = pump_monitor_id->valueint;
    cJSON *current_sensor_id = cJSON_GetObjectItem(pump_monitor, "current_sensor_id");
    if (!current_sensor_id || !cJSON_IsNumber(current_sensor_id))
    {
        ESP_LOGE(TAG, "pump monitor current sensor id returend empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->pump_monitor.current_senor_id = current_sensor_id->valueint;
    cJSON *pump_id = cJSON_GetObjectItem(pump_monitor, "pump_id");
    if (!pump_id || !cJSON_IsNumber(pump_id))
    {
        ESP_LOGE(TAG, "pump monitor pump id retured an empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->pump_monitor.pump_id = pump_id->valueint;

    // desrilized relay driver
    cJSON *relay = cJSON_GetObjectItem(root, "relay");
    if (!relay || !cJSON_IsObject(relay))
    {
        ESP_LOGE(TAG, "invalid relay object\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    cJSON *relay_id = cJSON_GetObjectItem(relay, "id");
    if (!relay_id || !cJSON_IsNumber(relay_id))
    {
        ESP_LOGE(TAG, "relay id returend an empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->relay.relay_id = relay_id->valueint;
    cJSON *pin_number = cJSON_GetObjectItem(relay, "pin_number");
    if (!pin_number || !cJSON_IsNumber(pin_number))
    {
        ESP_LOGE(TAG, "relay pin number retured an empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->relay.relay_pin_number = pin_number->valueint;

    // deserilized level sensor
    cJSON *level_sensor = cJSON_GetObjectItem(root, "level_sensor");
    if (!level_sensor || !cJSON_IsObject(level_sensor))
    {
        ESP_LOGE(TAG, " invalid level sensor object\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    cJSON *level_Sensor_id = cJSON_GetObjectItem(level_sensor, "id");
    if (!level_Sensor_id || !cJSON_IsNumber(level_Sensor_id))
    {
        ESP_LOGE(TAG, "level sensor id retured empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->level_sensor.level_sensor_id = level_Sensor_id->valueint;
    cJSON *interface = cJSON_GetObjectItem(level_sensor, "interface");
    if (!interface || !cJSON_IsString(interface))
    {
        ESP_LOGE(TAG, "level sensor interface retured empty/null string\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->level_sensor.interface = interface->valuestring;
    cJSON *sensor_addr = cJSON_GetObjectItem(level_sensor, "sensor_addr");
    if (!sensor_addr || !cJSON_IsNumber(sensor_addr))
    {
        ESP_LOGE(TAG, "level sensor addr returend empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->level_sensor.sensor_addr = sensor_addr->valueint;
    cJSON *protocol = cJSON_GetObjectItem(level_sensor, "protocol");
    if (!protocol || !cJSON_IsString(protocol))
    {
        ESP_LOGE(TAG, "level sensor protocol returend empty/null string\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->level_sensor.protocol = protocol->valuestring;

    // deserilized current sensor
    cJSON *current_sensor = cJSON_GetObjectItem(root, "current_sensor");
    if (!current_sensor || !cJSON_IsObject(current_sensor))
    {
        ESP_LOGE(TAG, "invalid current sensor object\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    cJSON *current_Sensor_id = cJSON_GetObjectItem(current_sensor, "id");
    if (!current_Sensor_id || !cJSON_IsNumber(current_Sensor_id))
    {
        ESP_LOGE(TAG, "current sensor id retured an empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->current_sensor.current_sensor_id = current_Sensor_id->valueint;
    cJSON *sensor_interface = cJSON_GetObjectItem(current_sensor, "interface");
    if (!sensor_interface || !cJSON_IsString(sensor_interface))
    {
        ESP_LOGE(TAG, "current sensor interface retured empty/null string\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->current_sensor.interface = sensor_interface->valuestring;
    cJSON *max_current = cJSON_GetObjectItem(current_sensor, "max_current");
    if (!max_current || !cJSON_IsNumber(max_current))
    {
        ESP_LOGE(TAG, "max current retured empty/null int\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->current_sensor.max_current = max_current->valueint;
    cJSON *make = cJSON_GetObjectItem(current_sensor, "make");
    if (!make || !cJSON_IsString(make))
    {
        ESP_LOGE(TAG, "current sensor make retured empty/null string\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    unit->current_sensor.current_sensor_make = make->valuestring;
     //cJSON_Delete(root);
    return SYSTEM_OK;
}

error_type_t setup_config_tank(pump_control_unit_t *pump_control_obj)
{

    if (pump_control_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    tank_shape_t _shape = string_to_tank_shape(pump_control_obj->tank.tank_shape);
    tank_config_t tank_config = {
        .id = pump_control_obj->tank.tank_id,
        .capacity_in_liters = pump_control_obj->tank.tank_capacity,
        .shape = _shape,
        .height_in_cm = pump_control_obj->tank.tank_height,
        .full_level_in_mm = pump_control_obj->tank.tank_full,
        .low_level_in_mm = pump_control_obj->tank.tank_low};
    tank_t *tank_obj = tank_create(tank_config);
    if (!tank_obj)
    {
        ESP_LOGE(TAG, "failed to create tank obj\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    return SYSTEM_OK;
}

error_type_t setup_config_pump(pump_control_unit_t *pump_control_obj)
{
    if (pump_control_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    pump_config_t pump_config = {
        .id = pump_control_obj->pump.pump_id,
        .make = pump_control_obj->pump.pump_make,
        .power_in_hp = pump_control_obj->pump.pump_power_in_hp,
        .current_rating = pump_control_obj->pump.pump_current_rating};

    pump_t *pump_obj = pump_create(pump_config);
    if (!pump_obj)
    {
        ESP_LOGE(TAG, "failed to create pump obj\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    return SYSTEM_OK;
}

error_type_t setup_config_tank_monitor(pump_control_unit_t *pump_control_obj)
{
    if (pump_control_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    level_sensor_config_t sensor_config = {
        .id = pump_control_obj->tank_monitor.level_sensor_id};
    level_sensor_t *sensor_obj = level_sensor_create(sensor_config);
    if (!sensor_obj)
    {
        ESP_LOGE(TAG, "invalid sensor obj\n.");
    }

    tank_config_t tank_config = {
        .id = pump_control_obj->tank_monitor.tank_id};
    tank_t *tank = tank_create(tank_config);
    if (!tank)
    {
        ESP_LOGE(TAG, "invaild tank \n.");
    }

    tank_monitor_config_t tank_monitor_config = {
        .id = pump_control_obj->tank_monitor.tank_monitor_id,
        .sensor = sensor_obj,
        .tank = tank};
    tank_monitor_t *tank_monitor_obj = tank_monitor_create(tank_monitor_config);
    if (!tank_monitor_obj)
    {
        ESP_LOGE(TAG, "invalid tank monitor object\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    return SYSTEM_OK;
}

error_type_t setup_config_pump_monitor(pump_control_unit_t *pump_control_obj)
{
    if (pump_control_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    current_sensor_config_t sensor = {
        .id = pump_control_obj->pump_monitor.current_senor_id};
    current_sensor_t *sensor_obj = current_sensor_create(&sensor);
    if (!sensor_obj)
    {
        ESP_LOGE(TAG, "invalid sensor obj\n.");
        return SYSTEM_INVALID_PARAMETER;
    }

    pump_config_t pump = {
        .id = pump_control_obj->pump_monitor.pump_id};
    pump_t *pump_obj = pump_create(pump);
    if (!pump_obj)
    {
        ESP_LOGE(TAG, "invalid pump object\n.");
        return SYSTEM_INVALID_PARAMETER;
    }

    pump_monitor_config_t pump_monitor_config = {
        .id = pump_control_obj->pump_monitor.pump_monitor_id,
        .sensor = sensor_obj,
        .pump = pump_obj};
    pump_monitor_t *pump_monitor = pump_monitor_create(pump_monitor_config);
    if (!pump_monitor)
    {
        ESP_LOGE(TAG, "invalid pump monitor\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    return SYSTEM_OK;
}

error_type_t setup_config_relay(pump_control_unit_t *pump_control_obj)
{
    if (pump_control_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    relay_config_t relay_config = {
        .id = pump_control_obj->relay.relay_id,
        .relay_pin_number = pump_control_obj->relay.relay_pin_number};
    relay_t *relay = relay_create(&relay_config);
    if (!relay)
    {
        ESP_LOGE(TAG, "invalid relay object");
        return SYSTEM_INVALID_PARAMETER;
    }
    return SYSTEM_OK;
}

static error_type_t set_level_sensor_interface_to_string(const char *config_str, level_sensor_config_t *config)
{
    if (strcmp("RS485", config_str) == 0)
    {
        config->medium_context = (void *)rs485Obj;
    }
    else if (strcmp("UART", config_str) == 0)
    {
        config->medium_context = (void *)rs485Obj;
    }
    else if (strcmp("PWM", config_str) == 0)
    {
        config->medium_context = (void *)rs485Obj;
    }
    else
    {
        ESP_LOGE(TAG, "Unknow interface");
        return SYSTEM_INVALID_PARAMETER;
    }
    return SYSTEM_OK;
}

static error_type_t set_level_sensor_protocol_to_string(const char *protocol_str, level_sensor_config_t *config)
{
    if (strcmp("GL_A01_PROTOCOL", protocol_str) == 0)
    {
        config->protocol = protocol_gl_a01_read_level;
    }
    else
    {
        ESP_LOGE(TAG, "Unknow protocol");
        return SYSTEM_INVALID_PARAMETER;
    }
    return SYSTEM_OK;
}

error_type_t setup_config_level_sensor(pump_control_unit_t *pump_control_obj)
{
    level_sensor_config_t level_sensor_config = {
        .id = pump_control_obj->level_sensor.level_sensor_id,
        .medium_context = (void *)NULL,
        .sensor_addr = pump_control_obj->level_sensor.sensor_addr,
        .protocol = NULL,
    };
    error_type_t err = set_level_sensor_interface_to_string(pump_control_obj->level_sensor.interface, &level_sensor_config);
    if (!err)
    {
        return SYSTEM_INVALID_PARAMETER;
    }
    
    err = set_level_sensor_protocol_to_string(pump_control_obj->level_sensor.protocol, &level_sensor_config);
    if (!err)
    {
        return SYSTEM_INVALID_PARAMETER;
    }
    
    level_sensor_t *level_sensor = level_sensor_create(level_sensor_config);
    if (!level_sensor)
    {
        ESP_LOGE(TAG, "invallid level sensor object\n.");
        return SYSTEM_INVALID_PARAMETER;
    }

    return SYSTEM_OK;
}

static error_type_t set_current_sensor_interface_to_string(const char *current_sensor_interface_str, current_sensor_config_t *config)
{
    if (strcmp("I2C", current_sensor_interface_str) == 0)
    {
        config->context = (void**)adc1115_obj;
    }
    else if (strcmp("SPI", current_sensor_interface_str) == 0)
    {
        config->context = (void**)adc1115_obj;
    }
    else if (strcmp("PWM", current_sensor_interface_str) == 0)
    {
        config->context = (void**)adc1115_obj;
    }
    else if (strcmp("ANALOG_VOLTAGE", current_sensor_interface_str) == 0)
    {
        config->context = (void**)adc1115_obj;
    }
    else
    {
        ESP_LOGE(TAG, "unknown current sensor interface");
        return SYSTEM_INVALID_PARAMETER;
    }
    return SYSTEM_OK;
}

error_type_t setup_config_current_sensor(pump_control_unit_t *pump_control_obj)
{
    current_sensor_config_t current_sensor_config = {
        .id = pump_control_obj->current_sensor.current_sensor_id,
        .context = (void **)NULL,
        .make = pump_control_obj->current_sensor.current_sensor_make,
        .max_current = pump_control_obj->current_sensor.max_current};
    set_current_sensor_interface_to_string(pump_control_obj->current_sensor.interface, &current_sensor_config);
    current_sensor_t *current_sensor = current_sensor_create(&current_sensor_config);
    if (!current_sensor)
    {
        ESP_LOGE(TAG, "invalid current sensor object\n.");
        return SYSTEM_INVALID_PARAMETER;
    }
    return SYSTEM_OK;
}
