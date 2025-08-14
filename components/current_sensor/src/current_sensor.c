#include <current_sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <common_headers.h>
#include <stddef.h>

struct current_sensor_t {
    current_sensor_config_t* config; // Configuration for the current sensor
    bool is_initialized; // Flag to check if the sensor is initialized
};

current_sensor_t* current_sensor_create(current_sensor_config_t* config) {
    current_sensor_t *sensor = (current_sensor_t *)malloc(sizeof(current_sensor_t));
    if (sensor == NULL) {
        return NULL; // Handle memory allocation failure
    }

    sensor->config = config; 
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
    if (sensor->config->id < 0 || sensor->config->read_current == NULL) {
        return SYSTEM_INVALID_PARAMETER; // Handle invalid configuration
    }
    sensor->is_initialized = true; // Set the initialized flag
    // Initialize the current sensor (if any specific actions are needed)
    printf("Current sensor initialized with ID: %d\n", sensor->config->id);
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
    printf("Current sensor deinitialized\n");
    return SYSTEM_OK;
}

error_type_t current_sensor_destroy(current_sensor_t **sensor) {
    if (*sensor == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    free(*sensor); // Free the allocated memory for the sensor
    *sensor = NULL; // Set the pointer to NULL after freeing
    printf("Current sensor destroyed\n");
    return SYSTEM_OK;
}

error_type_t current_sensor_get_current_in_amp(const current_sensor_t *sensor, float *current_value) {
    if (sensor == NULL || current_value == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor or reading pointer
    }
    if (!sensor->is_initialized) {
        return SYSTEM_INVALID_STATE; // Sensor is not initialized
    }
    printf("Getting current reading for sensor ID: %d\n", sensor->config->id);
    // Call the reading callback function to get the current value
    error_type_t err = sensor->config->read_current(*sensor->config->context, current_value);
    if (err != SYSTEM_OK) { 
        printf("Error reading current: %d\n", err);
        return err; // Handle error in reading current
    }
    printf("Current reading: %.2f A\n", *current_value);
    
    return SYSTEM_OK;
}


current_sensor_interface_t string_to_current_sensor_interface(const char* interface_str){
    if (strcmp(interface_str, "I2C") == 0)
    {
        return CURRENT_SENSOR_INTERFACE_I2C;
    }else if (strcmp(interface_str, "SPI") == 0)
    {
        return CURRENT_SENSOR_INTERFACE_SPI;
    }else if (strcmp(interface_str, "PWM") == 0)
    {
        return CURRENT_SENSOR_INTERFACE_PWM;
    }else if (strcmp(interface_str, "ANALOG_VOLTAGE")== 0)
    {
        return CURRENT_SENSOR_INTERFACE_ANALOG_VOLTAGE;
    }else
    {
        printf("Uknown");
        return CURRENT_SENSOR_INTERFACE_I2C;
    }
}   