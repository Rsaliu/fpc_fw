#ifndef __PUMP_LIBRARY_H__
#define __PUMP_LIBRARY_H__
#include <common_headers.h>
#include <stddef.h>

typedef struct{
    int id;
    char * make;
    float power_in_hp;
    float current_rating;
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
error_type_t pump_deinit(pump_t *pump);
error_type_t pump_destroy(pump_t **pump);
error_type_t pump_get_state(const pump_t *pump, pump_state_t *state);
error_type_t pump_get_config(const pump_t *pump, pump_config_t *config);
error_type_t pump_print_info(pump_t* pump);
error_type_t pump_print_info_into_buffer(pump_t* pump, char* buffer, const size_t buffer_size);

#endif