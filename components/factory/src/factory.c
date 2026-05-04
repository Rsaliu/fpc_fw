#include "factory.h"
#include <globals.h>
#include <current_sensor.h>
#include <esp_log.h>
#include <protocol.h>
#include <rs485_context.h>
#define PUMP_MONITOR_NUMBER_OF_SAMPLES_FOR_AVERAGE 10
static const gpio_num_t ALERT_READY_PIN_ADS1115_ONE= GPIO_NUM_26; // GPIO pin for ALERT/READY signal during testing
const static char* TAG = "FACTORY";
error_type_t setup_internal_adc_reader(adc_reader_t** adc_reader, adc_channel_t channel){
    adc_reader_config_t config = {
        .adc_unit_id = ADC_UNIT_1,
        .adc_channel = channel,
        .adc_atten = ADC_ATTEN_DB_11,
        .adc_bitwidth = ADC_BITWIDTH_12
    };
    *adc_reader = adc_reader_create(&config);
    if (*adc_reader == NULL) {
        return SYSTEM_FAILED;
    }
    error_type_t err = adc_reader_init(*adc_reader);
    if (err != SYSTEM_OK) {
        adc_reader_destroy(adc_reader);
        return err;
    }
    return SYSTEM_OK;
}

error_type_t setup_acs712_with_internal_adc(acs712_sensor_t** acs712_sensor, adc_reader_t* reader,acs712_read_mode_t read_mode){
    acs712_config_t config = {
        .context = (void*)reader, 
        .zero_voltage = 2500, // Zero voltage offset for the sensor
        .read_mode = read_mode,
        .adc_reader = (acs712_reading_callback_t)adc_reader_read
    };
    *acs712_sensor = acs712_create(&config);
    if (*acs712_sensor == NULL) {
        return SYSTEM_FAILED;
    }
    error_type_t err = acs712_sensor_init(*acs712_sensor);
    if (err != SYSTEM_OK) {
        acs712_destroy(acs712_sensor);
        return err; 
    }
    return SYSTEM_OK;
}

error_type_t setup_current_sensor_with_acs712(current_sensor_t** current_sensor, int id, acs712_sensor_t* acs712_sensor, current_sensor_read_mode_t read_mode){
    current_sensor_config_t config = {
        .id = id, // You can set this to a unique identifier as needed
        .make = "ACS712 with Internal ADC",
        .read_mode = read_mode,
        .sync_read_callback = (current_sensor_sync_read_callback_t)acs712_read_current,
        .callback_context = (void*)acs712_sensor
    };
    *current_sensor = current_sensor_create(&config);
    if (*current_sensor == NULL) {
        return SYSTEM_FAILED;
    }
    error_type_t err = current_sensor_init(*current_sensor);
    if (err != SYSTEM_OK) {
        current_sensor_destroy(current_sensor);
        return err; 
    }
    return SYSTEM_OK;
}


error_type_t setup_ads1115_oneshot_with_i2c(ads1115_t** ads1115,i2c_master_dev_handle_t dev_handle, ads1115_input_channel_t channel){
    ads1115_config_t ads_config_oneshot = {
    .alert_ready_pin = ALERT_READY_PIN_ADS1115_ONE,
    .i2c_dev_handle = dev_handle, // Get the I2C device handle from globals
    .input_channel = channel,
    .pga_mode = ADS1115_PGA_6_144V,   
    .measurement_mode = ADS1115_MEASUREMENT_ONE_SHOT,
    .comparator_callback = NULL,
    .measurement_callback = NULL,
    .callback_context_ = NULL
    };
    *ads1115 = ads1115_create(&ads_config_oneshot);
    if (*ads1115 == NULL) {
        return SYSTEM_FAILED;
    }
    error_type_t err = ads1115_init(*ads1115);
    if (err != SYSTEM_OK) {
        ads1115_destroy(ads1115);
        return err; 
    }
    return SYSTEM_OK;
}

error_type_t setup_acs712_with_ads1115(acs712_sensor_t** acs712_sensor, ads1115_t* ads1115, acs712_read_mode_t read_mode){
    acs712_config_t config = {
        .context = (void*)ads1115, 
        .zero_voltage = 2500, // Zero voltage offset for the sensor
        .read_mode = read_mode,
        .adc_reader = (acs712_reading_callback_t)ads1115_read_one_shot
    };
    *acs712_sensor = acs712_create(&config);
    if (*acs712_sensor == NULL) {
        return SYSTEM_FAILED;
    }
    error_type_t err = acs712_sensor_init(*acs712_sensor);
    if (err != SYSTEM_OK) {
        acs712_destroy(acs712_sensor);
        return err; 
    }
    return SYSTEM_OK;
}

error_type_t setup_current_sensor_from_config(current_sensor_setup_config_t* config, current_sensor_t** current_sensor){
    ads1115_t* ads1115 = NULL;
    acs712_sensor_t* acs712_sensor = NULL;
    if(config == NULL || current_sensor == NULL){
        return SYSTEM_NULL_PARAMETER;
    }
    // we only support basic read mode (synchronous) for now, other modes can be added later
    if(config->read_mode != CURRENT_SENSOR_CONFIG_READ_MODE_BASIC){
        return SYSTEM_INVALID_PARAMETER;
    }
    // for now we only support ACS712 with ADS1115, other combinations can be added later
    if(config->current_sensor_make != ACS712){
        return SYSTEM_INVALID_PARAMETER; // Unsupported make for ADS1115 interface
    }
    if(config->interface.interface == ADS1115_ONE){
        //print the channel for debugging
        ESP_LOGI(TAG, "Setting up current sensor with ADS1115 interface, channel: %d", config->interface.channel);
        error_type_t err = setup_ads1115_oneshot_with_i2c(&ads1115, globals_get_i2c_dev_handle_option(1), config->interface.channel);
        if(err != SYSTEM_OK){
            return err;
        }
        // print pointer value of globals_get_i2c_dev_handle_option(1)
        ESP_LOGI(TAG, "Pointer value of I2C device handle from globals: %p", (void*)globals_get_i2c_dev_handle_option(1));
        err = setup_acs712_with_ads1115(&acs712_sensor, ads1115, ACS712_READ_MODE_BASIC);
        if(err != SYSTEM_OK){
            return err;
        }
        return setup_current_sensor_with_acs712(current_sensor, config->current_sensor_id, acs712_sensor, CURRENT_SENSOR_READ_MODE_BASIC);
    }
    if(config->interface.interface == INTERNAL_ADC){
        adc_reader_t* adc_reader = NULL;
        error_type_t err = setup_internal_adc_reader(&adc_reader, config->interface.channel);
        if(err != SYSTEM_OK){
            return err;
        }
        err = setup_acs712_with_internal_adc(&acs712_sensor, adc_reader, ACS712_READ_MODE_BASIC);
        if(err != SYSTEM_OK){
            return err;
        }
        return setup_current_sensor_with_acs712(current_sensor, config->current_sensor_id, acs712_sensor, CURRENT_SENSOR_READ_MODE_BASIC);
    }
    return SYSTEM_INVALID_PARAMETER; // Unsupported interface
}

error_type_t setup_pump_from_config(pump_setup_config_t* config, pump_t** pump){
    if(config == NULL || pump == NULL){
        return SYSTEM_NULL_PARAMETER;
    }
    pump_config_t pump_config = {
        .id = config->pump_id,
        .make = config->pump_make,
        .power_in_hp = config->pump_power_in_hp,
        .current_rating = config->pump_current_rating,
        .min_working_current = config->pump_min_working_current
    };
    *pump = pump_create(pump_config);
    if (*pump == NULL) {
        return SYSTEM_FAILED;
    }
    error_type_t err = pump_init(*pump);
    if (err != SYSTEM_OK) {
        pump_destroy(pump);
        return err; 
    }
    return SYSTEM_OK;
}

error_type_t setup_pump_monitor_from_config(pump_monitor_setup_config_t* config, pump_monitor_t** pump_monitor, pump_t* pump, current_sensor_t* current_sensor, current_analytics_callback_t analytics_cb){
    if(config == NULL || pump_monitor == NULL || pump == NULL || current_sensor == NULL || analytics_cb == NULL){
        ESP_LOGE(TAG, "Invalid parameter(s) provided to setup_pump_monitor_from_config");
        return SYSTEM_NULL_PARAMETER;
    }
    current_sensor_config_t  current_sensor_config;
    // print pointer *current_sensor for debugging
    ESP_LOGI(TAG, "Pointer value of current_sensor in setup_pump_monitor_from_config: %p", current_sensor);
    error_type_t err = current_sensor_get_config(current_sensor, &current_sensor_config);
    if(err != SYSTEM_OK){
        ESP_LOGE(TAG, "Failed to get current sensor config, error code: %d", err);
        return err;
    }
    if(current_sensor_config.read_mode != CURRENT_SENSOR_READ_MODE_BASIC){
        ESP_LOGE(TAG, "Unsupported read mode for pump monitor setup: %d", current_sensor_config.read_mode);
        return SYSTEM_INVALID_PARAMETER; // We only support basic read mode for now
    }
    pump_monitor_config_t monitor_config = {
        .id = config->pump_monitor_id,
        .pump = pump,
        .sensor = current_sensor,
        .current_read_cb = current_sensor_get_current_in_amp, // We only support basic read mode for now
        .number_of_samples_for_average = PUMP_MONITOR_NUMBER_OF_SAMPLES_FOR_AVERAGE,
        .analytics_cb = analytics_cb
    };
    *pump_monitor = pump_monitor_create(monitor_config);
    if (*pump_monitor == NULL) {
        ESP_LOGE(TAG, "Failed to create pump monitor instance");
        return SYSTEM_FAILED;
    }
    err = pump_monitor_init(*pump_monitor);
    if (err != SYSTEM_OK) {
        ESP_LOGE(TAG, "Failed to initialize pump monitor instance, error code: %d", err);
        pump_monitor_destroy(pump_monitor);
        return err; 
    }
    return SYSTEM_OK;
}

error_type_t setup_relay_from_config(relay_setup_config_t* config, relay_t** relay){
    if(config == NULL || relay == NULL){
        return SYSTEM_NULL_PARAMETER;
    }
    relay_config_t relay_config = {
        .id = config->relay_id,
        .relay_pin_number = config->relay_pin_number
    };
    *relay = relay_create(&relay_config);
    if (*relay == NULL) {
        return SYSTEM_FAILED;
    }
    error_type_t err = relay_init(*relay);
    if (err != SYSTEM_OK) {
        relay_destroy(relay);
        return err; 
    }
    return SYSTEM_OK;
}

error_type_t setup_tank_from_config(tank_setup_config_t* config, tank_t** tank){
    if(config == NULL || tank == NULL){
        return SYSTEM_NULL_PARAMETER;
    }
    tank_config_t tank_config = {
        .id = config->tank_id,
        .capacity_in_liters = config->tank_capacity,
        .shape = config->tank_shape,
        .height_in_cm = config->tank_height_cm,
        .full_level_in_mm = config->tank_full_level_mm,
        .low_level_in_mm = config->tank_low_level_mm
    };
    *tank = tank_create(tank_config);
    if (*tank == NULL) {
        return SYSTEM_FAILED;
    }
    error_type_t err = tank_init(*tank);
    if (err != SYSTEM_OK) {
        tank_destroy(tank);
        return err; 
    }
    return SYSTEM_OK;
}

static protocol_interpreter_t get_interpreter_callback(level_sensor_protocol_type_t protocol){
    switch(protocol){
        case GA1:
            return protocol_gl_a01_interpreter;
        default:
            return NULL;
    }
}

static protocol_callback_t get_protocol_callback(level_sensor_protocol_type_t protocol){
    switch(protocol){
        case GA1:
            return protocol_gl_a01_read_level;
        default:
            return NULL;
    }
}

static send_receive_t get_send_receive_callback(level_sensor_interface_type_t interface){
    switch(interface){
        case RS485:
            return rs485_context_send_receive;
        default:
            return NULL;
    }
}

error_type_t setup_level_sensor_from_config(level_sensor_setup_config_t* config, level_sensor_t** level_sensor, void* medium_context){
    // This function can be implemented similarly to the others, based on the level sensor configuration structure and creation/init functions
    if(config == NULL || level_sensor == NULL || medium_context == NULL){
        return SYSTEM_NULL_PARAMETER;
    }
    level_sensor_config_t sensor_config = {
        .id = config->level_sensor_id,
        .sensor_addr = config->sensor_addr,
        .protocol = get_protocol_callback(config->protocol),
        .medium_context = medium_context, // This would also need to be set based on the interface specified in the config
        .send_recive = get_send_receive_callback(config->interface), // Set this based on the interface and protocol as needed
        .interpreter = get_interpreter_callback(config->protocol) // Set this based on the protocol as needed
    };
    *level_sensor = level_sensor_create(sensor_config);
    if (*level_sensor == NULL) {
        return SYSTEM_FAILED;
    }
    error_type_t err = level_sensor_init(*level_sensor);
    if (err != SYSTEM_OK) {
        level_sensor_destroy(level_sensor);
        return err; 
    }

    return SYSTEM_OK;
}


error_type_t setup_tank_monitor_from_config(tank_monitor_setup_config_t* config, tank_monitor_t** tank_monitor, tank_t* tank, level_sensor_t* level_sensor, level_analytics_callback_t analytics_cb){
    if(config == NULL || tank_monitor == NULL || tank == NULL || level_sensor == NULL || analytics_cb == NULL){
        ESP_LOGE(TAG, "Invalid parameter(s) provided to setup_tank_monitor_from_config");
        return SYSTEM_NULL_PARAMETER;
    }
    tank_monitor_config_t monitor_config = {
        .id = config->tank_monitor_id,
        .tank = tank,
        .sensor = level_sensor,
        .level_read_cb = (level_sensor_read_callback_t)level_sensor_read, // We only support basic read mode for now
        .number_of_samples_for_average = PUMP_MONITOR_NUMBER_OF_SAMPLES_FOR_AVERAGE, // This can be made configurable later
        .analytics_cb = analytics_cb
    };
    *tank_monitor = tank_monitor_create(monitor_config);
    if (*tank_monitor == NULL) {
        ESP_LOGE(TAG, "Failed to create tank monitor instance");
        return SYSTEM_FAILED;
    }
    error_type_t err = tank_monitor_init(*tank_monitor);
    if (err != SYSTEM_OK) {
        ESP_LOGE(TAG, "Failed to initialize tank monitor instance, error code: %d", err);
        tank_monitor_destroy(tank_monitor);
        return err; 
    }
     ESP_LOGI(TAG, "Tank monitor created and initialized with ID: %d\n", config->tank_monitor_id);
    return SYSTEM_OK;
}