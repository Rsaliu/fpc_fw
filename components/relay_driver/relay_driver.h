#ifndef __RELAY_DRIVER_H__
#define __RELAY_DRIVER_H__

#include <stdint.h>
#include <stdbool.h>
#include <common_headers.h>

typedef enum
{
    RELAY_OFF = 0,
    RELAY_ON
} relay_state_t;

typedef struct
{ 
    int id;
    uint8_t relay_pin_number;
} relay_config_t;

//
typedef struct relay_t relay_t;

relay_t *relay_create(const relay_config_t *config); // create a relay with pin number
error_type_t relay_get_config(const relay_t *relay,relay_config_t *config_out);
error_type_t relay_init(relay_t *relay);  // set up the relay pin for use
error_type_t relay_deinit(relay_t *relay);  // deinitialize relay
error_type_t relay_switch(relay_t *relay, relay_state_t state); // turn on or off
error_type_t relay_check_state(const relay_t *relay, relay_state_t* state); // check the state either on or off
error_type_t relay_destroy(relay_t **relay); // free allocated memory

#endif // RELAY_DRIVER_H