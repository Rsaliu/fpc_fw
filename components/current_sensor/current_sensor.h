#ifndef __CURRENT_SENSOR_H__
#define __CURRENT_SENSOR_H__
#include <common_headers.h>
#include <current_sensor_common.h>

typedef error_type_t (*current_sensor_sync_read_callback_t)(void* context, float* current_value);

typedef error_type_t (*current_sensor_overcurrent_monitor_callback_t)(void* sensor, float max_threshold_current, float min_threshold_current, overcurrent_comparator_callback_t callback,  void* context);

typedef error_type_t (*current_sensor_continuous_read_callback_t)(void* sensor, measurement_complete_callback_t callback,  void* context);

// typedef error_type_t (*raw_to_current_converter_t)(const current_sensor_t* sensor, const uint16_t adc_voltage, float* current);
typedef enum{
    CURRENT_SENSOR_READ_MODE_BASIC,
    CURRENT_SENSOR_READ_MODE_OVERCURRENT_MONITOR,
    CURRENT_SENSOR_READ_MODE_CONTINUOUS_MEASUREMENT
} current_sensor_read_mode_t;

typedef struct{
    int id; // Unique identifier for the current sensor
    char* make;
    current_sensor_read_mode_t read_mode; // Reading mode of the sensor
    current_sensor_sync_read_callback_t sync_read_callback; // Callback for synchronous reading
    void* callback_context; // Context for the read callback
    //void* raw_to_current_converter; // Function pointer to convert raw ADC value to current
}current_sensor_config_t;

typedef struct current_sensor_t current_sensor_t;
current_sensor_t* current_sensor_create(current_sensor_config_t* config);
error_type_t current_sensor_init(current_sensor_t *sensor);
error_type_t current_sensor_deinit(current_sensor_t *sensor);
error_type_t current_sensor_destroy(current_sensor_t **sensor);
error_type_t current_sensor_get_config(const current_sensor_t *sensor, current_sensor_config_t *config);


error_type_t current_sensor_get_current_in_amp(const current_sensor_t *sensor, float *current_value);
// error_type_t current_sensor_monitor_overcurrent(const current_sensor_t* sensor,float max_threshold_current,float min_threshold_current, overcurrent_comparator_callback_t callback, void* context,current_sensor_overcurrent_monitor_callback_t over_monitor_callback, void* overcurrent_monitor_context);
// error_type_t current_sensor_continuous_read(const current_sensor_t* sensor, measurement_complete_callback_t callback, void* context,current_sensor_continuous_read_callback_t continuous_read_callback, void* continuous_read_context);
// error_type_t current_sensor_get_current_from_raw_value(const current_sensor_t* sensor, const uint16_t adc_voltage, float* current);
#endif // __CURRENT_SENSOR_H__