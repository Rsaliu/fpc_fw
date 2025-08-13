#include <tank.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "esp_log.h"
typedef struct tank_t {
    tank_config_t *config; // Pointer to the tank configuration
    tank_state_t state; // State of the tank
} tank_t;

static const char* TAG = "TANK";

tank_t* tank_create(tank_config_t config){
    tank_t *tank = (tank_t *)malloc(sizeof(tank_t));
    if (tank == NULL) {
        return NULL; // Handle memory allocation failure
    }

    tank->config = (tank_config_t *)malloc(sizeof(tank_config_t));
    if (tank->config == NULL) {
        free(tank);
        return NULL; // Handle memory allocation failure
    }
    
    memcpy(tank->config, &config, sizeof(tank_config_t));
    tank->state = TANK_NOT_INITIALIZED;
    return tank;
}
error_type_t tank_init(tank_t *tank){
    if (tank == NULL || tank->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null tank or configuration
    }
    // Validate the tank configuration
    if (tank->config->id < 0 || tank->config->capacity_in_liters < 0.0 || 
        (tank->config->shape != TANK_SHAPE_RECTANGLE && tank->config->shape != TANK_SHAPE_CYLINDER) ||
        tank->config->height_in_cm <= 0 || tank->config->full_level_in_mm < 0 || 
        tank->config->low_level_in_mm < 0 || tank->config->full_level_in_mm <= tank->config->low_level_in_mm) {
        return SYSTEM_INVALID_PARAMETER; // Handle invalid configuration
    }
    if (tank->state != TANK_NOT_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Tank is already initialized
    }

    tank->state = TANK_INITIALIZED;
    ESP_LOGI(TAG,"Tank initialized with ID: %d, Capacity: %.2f liters, Shape: %d\n",
           tank->config->id, tank->config->capacity_in_liters, tank->config->shape);

    return SYSTEM_OK;
}
error_type_t tank_deinit(tank_t *tank){
    if (tank == NULL || tank->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null tank or configuration
    }

    if (tank->state == TANK_NOT_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Tank is not initialized
    }

    // Deinitialize the tank (if any specific actions are needed)
    tank->state = TANK_NOT_INITIALIZED;
    ESP_LOGI(TAG,"Tank deinitialized\n");

    return SYSTEM_OK;
}

error_type_t tank_destroy(tank_t **tank){
    if (*tank == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null tank
    }
    
    free((*tank)->config); // Free the allocated memory for the tank configuration
    (*tank)->config = NULL; // Set the pointer to NULL after freeing
    // Free the allocated memory for the tank
    free(*tank);
    *tank = NULL; // Set the pointer to NULL after freeing
    ESP_LOGI(TAG,"Tank destroyed\n");

    return SYSTEM_OK;
}
error_type_t tank_get_config(const tank_t *tank, tank_config_t *config){
    if (tank == NULL || config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null tank or configuration pointer
    }

    // Copy the tank configuration to the provided config pointer
    memcpy(config, tank->config, sizeof(tank_config_t));

    return SYSTEM_OK;
}

error_type_t tank_get_state(const tank_t *tank, tank_state_t *state){
    if (tank == NULL || state == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null tank or state pointer
    }
    
    *state = tank->state; // Get the current state of the tank
    ESP_LOGI(TAG,"Tank state retrieved: %d\n", *state);
    
    return SYSTEM_OK;
}
// Convert tank shape enum to string
const char* shape_to_string(tank_shape_t shape) {
    switch (shape) {
        case TANK_SHAPE_CYLINDER: return "Cylinder";
        case TANK_SHAPE_RECTANGLE: return "Rectangle";
        default: return "Unknown";
    }
}

//convert string to tank_shape
tank_shape_t string_to_tank_shape(const char* str_shape){
    if (strcmp(str_shape, "Cylinder") == 0)
    {
        return TANK_SHAPE_CYLINDER;
    } else if (strcmp(str_shape, "Rectangle") == 0)
    {
        return TANK_SHAPE_RECTANGLE;
    }else
    {
          ESP_LOGW(TAG, "Unknown shape");
          return TANK_SHAPE_RECTANGLE;
    }
}

// Print all tank details
error_type_t tank_print_info(tank_t *tank) {
ESP_LOGI(TAG,"ID: %d\n", tank->config->id);
ESP_LOGI(TAG,"Capacity:%.2f\n",tank->config->capacity_in_liters);
ESP_LOGI(TAG,"Height:%.2f\n",tank->config->height_in_cm);
ESP_LOGI(TAG,"Full level: %d\n", tank->config->full_level_in_mm);
ESP_LOGI(TAG,"Low level: %d\n", tank->config->low_level_in_mm);
ESP_LOGI(TAG,"Shape: %s\n", shape_to_string(tank->config->shape));
    return SYSTEM_OK; // Assuming you define ERROR_NONE in your enum
}
error_type_t tank_print_info_to_buffer(tank_t *tank, char* buffer, const size_t buffer_size){
    int written = snprintf(buffer, buffer_size,
        "Tank ID: %d\n Capacity: %.2f liters\n Height: %.2f cm\n Low Level: %d cm\nHigh Level: %d cm\nShape: %s\n",
        tank->config->id,
        tank->config->capacity_in_liters,
        tank->config->height_in_cm,
        tank->config->low_level_in_mm,
        tank->config->full_level_in_mm,
        shape_to_string(tank->config->shape));
    if (written < 0)return SYSTEM_OPERATION_FAILED;
        
    if ((size_t)written > buffer_size) {
        return SYSTEM_BUFFER_OVERFLOW;
    }
    
return SYSTEM_OK;
    
}
