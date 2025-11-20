#ifndef __CURRENT_SENSOR_H__
#define __CURRENT_SENSOR_H__
#include <common_headers.h>


typedef error_type_t (*current_sensor_reading_callback_t)(void* context, float* current_value);

 

typedef struct{
    int id; // Unique identifier for the current sensor
    void** context; // Context for the callback, can be used to pass additional data
    current_sensor_reading_callback_t read_current; // Function pointer to read current
    char* make;
    int max_current;
}current_sensor_config_t;

typedef struct current_sensor_t current_sensor_t;
current_sensor_t* current_sensor_create(current_sensor_config_t* config);
error_type_t current_sensor_init(current_sensor_t *sensor);
error_type_t current_sensor_deinit(current_sensor_t *sensor);
error_type_t current_sensor_destroy(current_sensor_t **sensor);
error_type_t current_sensor_get_config(const current_sensor_t *sensor, current_sensor_config_t *config);
error_type_t current_sensor_get_current_in_amp(const current_sensor_t *sensor, float *current);

#endif // __CURRENT_SENSOR_H__