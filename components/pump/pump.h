#ifndef __PUMP_LIBRARY_H__
#define __PUMP_LIBRARY_H__
#include <common_headers.h>

// dummy relay for pump control
typedef struct {
    int pin_number; // GPIO pin number
} relay_t;
//

typedef struct{
    int id;
    char * make;
    float power_in_hp; 
    relay_t * relay; // Relay to control the pump
} pump_config_t;


typedef enum {
    PUMP_NOT_INITIALIZED = 0,
    PUMP_ON = 1,
    PUMP_OFF = 2,
    PUMP_INITIALIZED = 3,
} pump_state_t;

typedef struct pump_t pump_t;


pump_t* pump_create(pump_config_t config);
error_type_t pump_init(pump_t *pump);
error_type_t pump_start(pump_t *pump);
error_type_t pump_stop(pump_t *pump);
error_type_t pump_deinit(pump_t *pump);
error_type_t pump_destroy(pump_t **pump);
error_type_t pump_get_state(pump_t *pump, pump_state_t *state);

#endif