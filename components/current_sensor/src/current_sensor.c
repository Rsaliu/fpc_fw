#include <current_sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <common_headers.h>
#include <stddef.h>
#include "esp_log.h"

static const char* TAG = "CURRENT_SENSOR";
struct current_sensor_t {
    current_sensor_config_t config; // Configuration for the current sensor
    bool is_initialized; // Flag to check if the sensor is initialized
};

current_sensor_t* current_sensor_create(current_sensor_config_t* config) {
    current_sensor_t *sensor = (current_sensor_t *)malloc(sizeof(current_sensor_t));
    if (sensor == NULL) {
        return NULL; // Handle memory allocation failure
    }
 
    // Deep copy the configuration to ensure the sensor has its own copy
    memcpy(&sensor->config, config, sizeof(current_sensor_config_t));
    sensor->is_initialized = false; // Initially not initialize
    return sensor;
}

error_type_t current_sensor_init(current_sensor_t *sensor) {
    if (sensor == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    if (sensor->is_initialized) {
        return SYSTEM_INVALID_STATE; // Sensor is already initialized
    }
    // Validate the sensor configuration
    if (sensor->config.id < 0  || sensor->config.make == NULL) {
        return SYSTEM_INVALID_PARAMETER; // Handle invalid configuration
    }
    sensor->is_initialized = true; // Set the initialized flag
    // Initialize the current sensor (if any specific actions are needed)
    ESP_LOGI(TAG, "Current sensor initialized with ID: %d\n", sensor->config.id);
    return SYSTEM_OK;
}

error_type_t current_sensor_deinit(current_sensor_t *sensor) {
    if (sensor == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    if (!sensor->is_initialized) {
        return SYSTEM_INVALID_STATE; // Sensor is not initialized
    }
    sensor->is_initialized = false; // Reset the initialized flag
    ESP_LOGI(TAG, "Current sensor deinitialized");
    return SYSTEM_OK;
}

error_type_t current_sensor_destroy(current_sensor_t **sensor) {
    if (*sensor == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    free(*sensor); // Free the allocated memory for the sensor
    *sensor = NULL; // Set the pointer to NULL after freeing
    ESP_LOGI(TAG, "Current sensor destroyed");
    return SYSTEM_OK;
}

error_type_t current_sensor_get_config(const current_sensor_t *sensor, current_sensor_config_t *config) {
    if (sensor == NULL || config == NULL) {
        return SYSTEM_NULL_PARAMETER;
    }
    if (!sensor->is_initialized) {
        return SYSTEM_INVALID_STATE;
    }

    memcpy(config, &sensor->config, sizeof(current_sensor_config_t));
    return SYSTEM_OK;
}

error_type_t current_sensor_get_current_in_amp(const current_sensor_t *sensor, float *current_value) {
    if (sensor == NULL || current_value == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor or reading pointer
    }
    if (!sensor->is_initialized) {
        return SYSTEM_INVALID_STATE; // Sensor is not initialized
    }
    if(sensor->config.read_mode != CURRENT_SENSOR_READ_MODE_BASIC){
        ESP_LOGE(TAG, "Sensor not in basic read mode");
        return SYSTEM_INVALID_MODE;
    }
    ESP_LOGI(TAG, "Getting current reading for sensor ID: %d", sensor->config.id);
    if(!sensor->config.sync_read_callback){
        ESP_LOGE(TAG, "Reading callback function not set");
        return SYSTEM_NULL_PARAMETER;
    }
    // Call the reading callback function to get the current value
    error_type_t err = sensor->config.sync_read_callback(sensor->config.callback_context, current_value);
    if (err != SYSTEM_OK) { 
        ESP_LOGE(TAG, "Error reading current: %d\n", err);
        return err; // Handle error in reading current
    }
    ESP_LOGI(TAG, "Current reading: %.2f A\n", *current_value);
    
    return SYSTEM_OK;
}


//error_type_t current_sensor_monitor_overcurrent(const current_sensor_t* sensor,float max_threshold_current,float min_threshold_current, overcurrent_comparator_callback_t callback, void* context){
// error_type_t current_sensor_monitor_overcurrent(const current_sensor_t* sensor,float max_threshold_current,float min_threshold_current, overcurrent_comparator_callback_t callback, void* context,current_sensor_overcurrent_monitor_callback_t over_monitor_callback, void* overcurrent_monitor_context){
    
//     if (sensor == NULL || !sensor->config) {
//         return SYSTEM_NULL_PARAMETER; // Handle null sensor
//     }

//     if (!sensor->is_initialized) {
//         return SYSTEM_INVALID_STATE; // Sensor is not initialized
//     }

//     if(sensor->config->read_mode != CURRENT_SENSOR_READ_MODE_OVERCURRENT_MONITOR){
//         ESP_LOGE(TAG, "Sensor not in overcurrent monitor mode");
//         return SYSTEM_INVALID_MODE;
//     }
    
//     ESP_LOGI(TAG, "Setting up overcurrent monitoring for sensor ID: %d with threshold: %.2f A", sensor->config->id, max_threshold_current);
//     ESP_LOGI(TAG, "Setting up undercurrent monitoring for sensor ID: %d with threshold: %.2f A", sensor->config->id, min_threshold_current);
//     // Call the overcurrent monitoring function from the sensor configuration
//     if(over_monitor_callback == NULL){
//         ESP_LOGE(TAG, "Overcurrent monitoring callback function not set");
//         return SYSTEM_NULL_PARAMETER;
//     }
//     error_type_t err = overcurrent_monitor_cb(overcurrent_monitor_context,max_threshold_current,min_threshold_current,callback, context);
//     if (err != SYSTEM_OK) {
//         ESP_LOGE(TAG, "Error setting up overcurrent monitoring: %d\n", err);
//         return err; // Handle error in setting up overcurrent monitoring    
//     }
//     return SYSTEM_OK;
// }

// error_type_t current_sensor_continuous_read(const current_sensor_t* sensor, measurement_complete_callback_t callback, void* context,current_sensor_continuous_read_callback_t continuous_read_callback, void* continuous_read_context)
// {
//     if (sensor == NULL || !sensor->config) {
//         return SYSTEM_NULL_PARAMETER; // Handle null sensor
//     }
//     if (!sensor->is_initialized) {
//         return SYSTEM_INVALID_STATE; // Sensor is not initialized
//     }
//     if(sensor->config->read_mode != CURRENT_SENSOR_READ_MODE_CONTINUOUS_MEASUREMENT){
//         ESP_LOGE(TAG, "Sensor not in continuous measurement mode");
//         return SYSTEM_INVALID_MODE;
//     }
//     ESP_LOGI(TAG, "Setting up continuous reading for sensor ID: %d", sensor->config->id);
//     // Call the continuous read function from the sensor configuration
//     if(continuous_read_callback == NULL){
//         ESP_LOGE(TAG, "Continuous read callback function not set");
//         return SYSTEM_NULL_PARAMETER;
//     }
//     error_type_t err = continuous_read_cb(continuous_read_context, callback, context);
//     if (err != SYSTEM_OK) {
//         ESP_LOGE(TAG, "Error setting up continuous reading: %d\n", err);
//         return err; // Handle error in setting up continuous reading    
//     }
//     return SYSTEM_OK;
// }

// error_type_t current_sensor_get_current_from_raw_value(const current_sensor_t* sensor, const uint16_t adc_voltage, float* current){
//     if (sensor == NULL || !sensor->config) {
//         return SYSTEM_NULL_PARAMETER; // Handle null sensor
//     }
//     if (!sensor->is_initialized) {
//         return SYSTEM_INVALID_STATE; // Sensor is not initialized
//     }
//     if(sensor->config->raw_to_current_converter == NULL){
//         ESP_LOGE(TAG, "Raw to current converter function not set");
//         return SYSTEM_NULL_PARAMETER;
//     }
//     // Call the raw to current conversion function from the sensor configuration
//     raw_to_current_converter_t converter_func = (raw_to_current_converter_t)sensor->config->raw_to_current_converter;
//     error_type_t err = converter_func(sensor, adc_voltage, current);
//     if (err != SYSTEM_OK) {
//         ESP_LOGE(TAG, "Error converting raw value to current: %d\n", err);
//         return err; // Handle error in conversion
//     }
//     ESP_LOGI(TAG, "Converted current reading: %.2f A from ADC value: %d\n", *current, adc_voltage);
//     return SYSTEM_OK;
// }
 