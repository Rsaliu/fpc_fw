#include <pump.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct pump_t {
    pump_config_t* config;
    pump_state_t state; // 0 for off, 1 for on
};


pump_t* pump_create(pump_config_t config){

    
    pump_t *pump = (pump_t *)malloc(sizeof(pump_t));
    if (pump == NULL) {
        return NULL; // Handle memory allocation failure
    }
    pump->config = (pump_config_t *)malloc(sizeof(pump_config_t));
    if (pump->config == NULL) {
        free(pump); // Free previously allocated memory
        return NULL; // Handle memory allocation failure
    }
    // Copy the configuration data into the pump's config
    memcpy(pump->config, &config, sizeof(pump_config_t));
    pump->state = PUMP_NOT_INITIALIZED;
    return pump;
}


error_type_t pump_init(pump_t *pump){
    if (pump == NULL || pump->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null pump or configuration
    }

    // validate the configuration
    if (pump->config->id < 0 || pump->config->power_in_hp < 0.0 || pump->config->make == NULL) {
        return SYSTEM_INVALID_PARAMETER; // Handle invalid configuration
    }
    
    if (pump->state != PUMP_NOT_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Pump is already initialized
    }
    
    pump->state = PUMP_INITIALIZED;
    printf("Pump initialized with ID: %d, Make: %s, Power: %.2f HP\n",
           pump->config->id, pump->config->make, pump->config->power_in_hp);
    
    return SYSTEM_OK;
}

error_type_t pump_deinit(pump_t *pump){
    if (pump == NULL || pump->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null pump or configuration
    }
    
    if (pump->state == PUMP_NOT_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Pump is not initialized
    }

    pump->state = PUMP_NOT_INITIALIZED;
    printf("Pump deinitialized\n");
    
    return SYSTEM_OK;
}
error_type_t pump_destroy(pump_t **pump){
    if (*pump == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null pump
    }
    free((*pump)->config); // Free the allocated memory for the pump configuration
    (*pump)->config = NULL; // Set the pointer to NULL after freeing
    // Free the allocated memory for the pump
    free(*pump);
    *pump = NULL; // Set the pointer to NULL after freeing
    printf("Pump destroyed\n");
    
    return SYSTEM_OK;
}

error_type_t pump_get_state(const pump_t *pump, pump_state_t *state){
    if (pump == NULL || state == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null pump or state pointer
    }
    
    *state = pump->state; // Get the current state of the pump
    printf("Pump state retrieved: %d\n", *state);
    
    return SYSTEM_OK;
}


error_type_t pump_get_config(const pump_t *pump, pump_config_t *config){
    if (pump == NULL || config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null pump or configuration pointer
    }
    
    // Copy the pump configuration to the provided config pointer
    memcpy(config, pump->config, sizeof(pump_config_t));
    
    return SYSTEM_OK;
}

error_type_t pump_print_info(pump_t* pump){
    printf("ID: %d\n", pump->config->id);
    printf("Make: %s\n", pump->config->make);
    printf("HP-Power: %f\n", pump->config->power_in_hp);
    printf("State: %d\n", pump->state);
    return SYSTEM_OK;
}

error_type_t pump_print_info_into_buffer(pump_t* pump, char* buffer, const size_t buffer_size){
    int written = snprintf(buffer, buffer_size, "Pump ID: %d\n Pump Make: %s\n HP Power: %f\n State: %d\n",
        pump->config->id, pump->config->make, pump->config->power_in_hp, pump->state);
        if (written >= 0 && (size_t)written < buffer_size)
        {
            return SYSTEM_BUFFER_OVERFLOW;
        }

        return SYSTEM_OK;       
}