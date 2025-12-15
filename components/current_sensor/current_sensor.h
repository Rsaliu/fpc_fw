#ifndef __CURRENT_SENSOR_H__
#define __CURRENT_SENSOR_H__
#include <common_headers.h>
#include <current_sensor_common.h>

typedef error_type_t (*current_sensor_reading_callback_t)(void* context, float* current_value);

typedef error_type_t (*current_sensor_overcurrent_monitor_callback_t)(void* sensor, float max_threshold_current, float min_threshold_current, overcurrent_comparator_callback_t callback,  void* context);

typedef error_type_t (*current_sensor_continuous_read_callback_t)(void* sensor, measurement_complete_callback_t callback,  void* context);

typedef enum{
    CURRENT_SENSOR_READ_MODE_BASIC,
    CURRENT_SENSOR_READ_MODE_OVERCURRENT_MONITOR,
    CURRENT_SENSOR_READ_MODE_CONTINUOUS_MEASUREMENT
} current_sensor_read_mode_t;

typedef struct{
    int id; // Unique identifier for the current sensor
    void** context; // Context for the callback, can be used to pass additional data
    char* make;
    int max_current;
    int min_current;
    void* callback;
    current_sensor_read_mode_t read_mode; // Reading mode of the sensor
}current_sensor_config_t;

typedef struct current_sensor_t current_sensor_t;
current_sensor_t* current_sensor_create(current_sensor_config_t* config);
error_type_t current_sensor_init(current_sensor_t *sensor);
error_type_t current_sensor_deinit(current_sensor_t *sensor);
error_type_t current_sensor_destroy(current_sensor_t **sensor);
error_type_t current_sensor_get_config(const current_sensor_t *sensor, current_sensor_config_t *config);
error_type_t current_sensor_get_current_in_amp(const current_sensor_t *sensor, float *current);
error_type_t current_sensor_monitor_overcurrent(const current_sensor_t* sensor, overcurrent_comparator_callback_t callback, void* context);
error_type_t current_sensor_continuous_read(const current_sensor_t* sensor, measurement_complete_callback_t callback, void* context);

#endif // __CURRENT_SENSOR_H__