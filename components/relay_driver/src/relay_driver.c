#include "driver/gpio.h"
#include "relay_driver.h"
#include <stdlib.h>
#include <esp_log.h>


// Relay object structure
struct relay_t
{
    int id;
    uint8_t pin;
    bool initialized;
    relay_state_t state;
};

// Create and allocate memory for relay object
relay_t *relay_create(const relay_config_t *config)
{
    if (config == NULL)
    {
        return NULL;
    }

    relay_t *relay = (relay_t *)malloc(sizeof(relay_t));
    if (relay == NULL)
    {
        return NULL;
    }

    relay->pin = config->relay_pin_number;
    relay->id = config->id;
    relay->initialized = false;
    relay->state = RELAY_OFF;

    return relay;
}
error_type_t relay_get_config(const relay_t *relay, relay_config_t *config_out)
{
    if (relay == NULL || config_out == NULL)
    {
      return SYSTEM_NULL_PARAMETER;  
    }
    (*config_out).relay_pin_number = relay->pin;
    (*config_out).id = relay->id;

    return SYSTEM_OK;
}
// Initialize relay GPIO pin (simplified version)
error_type_t relay_init(relay_t *relay)
{
    if (relay == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    if(relay->id < 0){
        return SYSTEM_INVALID_PARAMETER;
    }
    // Reset and set the pin as output
    gpio_reset_pin(relay->pin);                       // Clears any previous settings
    gpio_set_direction(relay->pin, GPIO_MODE_OUTPUT); // Set pin to output mode

    gpio_set_level(relay->pin, 0); // Set pin LOW (relay OFF)

    relay->initialized = true;
    relay->state = RELAY_OFF;

    return SYSTEM_OK;
}


error_type_t relay_deinit(relay_t *relay){
    if (relay == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    if (!relay->initialized)
    {
        return SYSTEM_INVALID_STATE;
    }
    relay->initialized = false;
    gpio_set_level(relay->pin, 0); // Set pin LOW (relay OFF)
    relay->state = RELAY_OFF;
    return SYSTEM_OK;

}


error_type_t relay_check_state(const relay_t *relay, relay_state_t *state)
{
    if (relay == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    if (!relay->initialized)
    {
        return SYSTEM_INVALID_STATE;
    }
       *state = relay->state;
    return SYSTEM_OK;
}

error_type_t relay_destroy(relay_t **relay)
{
    if (*relay == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    free(*relay);
    *relay = NULL;
    return SYSTEM_OK;
}

error_type_t relay_trip(relay_t *relay){
    if (relay == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    if (!relay->initialized)
    {
        return SYSTEM_INVALID_STATE;
    }
    if(relay->state == RELAY_TRIPPED){
        ESP_LOGW("RELAY", "Attempting to trip relay with ID %d that is already tripped. No action taken.", relay->id);
        return SYSTEM_OK; // Relay is already tripped, no action needed
    }
    gpio_set_level(relay->pin, 0); // Set pin LOW (relay OFF)
    relay->state = RELAY_TRIPPED;
    return SYSTEM_OK;
}

error_type_t relay_on(relay_t *relay){
    if (relay == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    if (!relay->initialized)
    {
        return SYSTEM_INVALID_STATE;
    }
    if(relay->state == RELAY_TRIPPED){
        ESP_LOGW("RELAY", "Attempting to turn ON relay with ID %d that is currently tripped. Please reset the relay before turning it ON.", relay->id);
        return SYSTEM_INVALID_STATE;
    }
    gpio_set_level(relay->pin, 1); // Set pin HIGH (relay ON)
    relay->state = RELAY_ON;
    return SYSTEM_OK;
}
error_type_t relay_off(relay_t *relay){
    if (relay == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    if (!relay->initialized)
    {
        return SYSTEM_INVALID_STATE;
    }
    if(relay->state == RELAY_TRIPPED){
        ESP_LOGW("RELAY", "Attempting to turn OFF relay with ID %d that is currently tripped. Please reset the relay before turning it OFF.", relay->id);
        return SYSTEM_INVALID_STATE;
    }
    gpio_set_level(relay->pin, 0); // Set pin LOW (relay OFF)
    relay->state = RELAY_OFF;
    return SYSTEM_OK;
}

error_type_t relay_reset(relay_t *relay){
    if (relay == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    if (!relay->initialized)
    {
        return SYSTEM_INVALID_STATE;
    }
    gpio_set_level(relay->pin, 0); // Set pin LOW (relay OFF)
    relay->state = RELAY_OFF;
    return SYSTEM_OK;
}

error_type_t relay_reset_and_on(relay_t *relay){
    if (relay == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    if (!relay->initialized)
    {
        return SYSTEM_INVALID_STATE;
    }
    gpio_set_level(relay->pin, 1); // Set pin HIGH (relay ON)
    relay->state = RELAY_ON;
    return SYSTEM_OK;
}
