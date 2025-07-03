#include <pump.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// dummy relay for pump control

error_type_t relay_switch_on(relay_t *relay){
    printf("Relay on pin switched ON\n");
    return SYSTEM_OK; // Simulate successful relay switch on
}
error_type_t relay_switch_off(relay_t *relay){
    printf("Relay on pin switched OFF\n");
    return SYSTEM_OK; // Simulate successful relay switch off
}
//

struct pump_t {
    pump_config_t* config;
    pump_state_t state; // 0 for off, 1 for on
};


pump_t* pump_create(pump_config_t config){

    
    pump_t *pump = (pump_t *)malloc(sizeof(pump_t));
    if (pump == NULL) {
        return NULL; // Handle memory allocation failure
    }

    pump->config = &config; // Assign the configuration to the pump
    pump->state = PUMP_NOT_INITIALIZED;
    return pump;
}


error_type_t pump_init(pump_t *pump){
    if (pump == NULL || pump->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null pump or configuration
    }
    
    if (pump->state != PUMP_NOT_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Pump is already initialized
    }
    
    // Initialize the relay for pump control
    if (pump->config->relay == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null relay
    }
    
    pump->state = PUMP_INITIALIZED;
    printf("Pump initialized with ID: %d, Make: %s, Power: %.2f HP\n",
           pump->config->id, pump->config->make, pump->config->power_in_hp);
    
    return SYSTEM_OK;
}


error_type_t pump_start(pump_t *pump){
    if (pump == NULL || pump->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null pump or configuration
    }
    
    if (pump->state != PUMP_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Pump must be initialized before starting
    }
    
    // Switch on the relay to start the pump
    error_type_t relay_status = relay_switch_on(pump->config->relay);
    if (relay_status != SYSTEM_OK) {
        return relay_status; // Handle relay switch on failure
    }
    
    pump->state = PUMP_ON;
    printf("Pump started\n");
    
    return SYSTEM_OK;
}

error_type_t pump_stop(pump_t *pump){
    if (pump == NULL || pump->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null pump or configuration
    }
    
    if (pump->state != PUMP_ON) {
        return SYSTEM_INVALID_STATE; // Pump must be running to stop
    }
    
    // Switch off the relay to stop the pump
    error_type_t relay_status = relay_switch_off(pump->config->relay);
    if (relay_status != SYSTEM_OK) {
        return relay_status; // Handle relay switch off failure
    }
    
    pump->state = PUMP_OFF;
    printf("Pump stopped\n");
    
    return SYSTEM_OK;
}
error_type_t pump_deinit(pump_t *pump){
    if (pump == NULL || pump->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null pump or configuration
    }
    
    if (pump->state == PUMP_NOT_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Pump is not initialized
    }
    
    // Deinitialize the pump by switching off the relay
    error_type_t relay_status = relay_switch_off(pump->config->relay);
    if (relay_status != SYSTEM_OK) {
        return relay_status; // Handle relay switch off failure
    }
    
    pump->state = PUMP_NOT_INITIALIZED;
    printf("Pump deinitialized\n");
    
    return SYSTEM_OK;
}
error_type_t pump_destroy(pump_t **pump){
    if (*pump == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null pump
    }
    
    // Free the allocated memory for the pump
    free(*pump);
    *pump = NULL; // Set the pointer to NULL after freeing
    printf("Pump destroyed\n");
    
    return SYSTEM_OK;
}

error_type_t pump_get_state(pump_t *pump, pump_state_t *state){
    if (pump == NULL || state == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null pump or state pointer
    }
    
    *state = pump->state; // Get the current state of the pump
    printf("Pump state retrieved: %d\n", *state);
    
    return SYSTEM_OK;
}