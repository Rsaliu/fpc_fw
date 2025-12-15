#ifndef __ACS712_CURRENT_SENSOR_H__
#define __ACS712_CURRENT_SENSOR_H__


#include <stdint.h>
#include <stdbool.h>
#include <esp_adc/adc_oneshot.h>
#include <common_headers.h>
#include <current_sensor_common.h>

typedef error_type_t (*acs712_reading_callback_t)(void* context, int* adc_voltage);

typedef error_type_t (*overcurrent_monitor_func_callback_t)(void* ads, const uint16_t high_threshold_value_in_millivolt, const uint16_t low_threshold_value_in_millivolt, overcurrent_comparator_callback_t comparator_callback, void* context);

typedef error_type_t (*acs712_measurement_complete_callback_t)(void* ads, measurement_complete_callback_t measurement_callback, void* context);

typedef enum{
    ACS712_READ_MODE_BASIC,
    ACS712_READ_MODE_OVERCURRENT_MONITOR,
    ACS712_READ_MODE_CONTINUOUS_MEASUREMENT
} acs712_read_mode_t;
typedef struct{
    void** context; // Context for the callback, can be used to pass additional data
    int zero_voltage; // Zero voltage offset for the sensor
    void* callback_func; // Placeholder for future callback functions   
    acs712_read_mode_t read_mode; // Reading mode of the sensor

}acs712_config_t;


typedef struct acs712_sensor_t acs712_sensor_t;

acs712_sensor_t* acs712_create(acs712_config_t* config);
error_type_t acs712_sensor_init(acs712_sensor_t* sensor);
error_type_t acs712_sensor_deinit(acs712_sensor_t* sensor);
error_type_t acs712_destroy(acs712_sensor_t** sensor);
error_type_t acs712_read_current(const acs712_sensor_t* sensor, float* current);
error_type_t acs712_monitor_current_window(const acs712_sensor_t* sensor, float max_threshold_current, float min_threshold_current, overcurrent_comparator_callback_t callback, void* context);
error_type_t acs712_monitor_read_current_with_cb(const acs712_sensor_t* sensor, measurement_complete_callback_t callback, void* context);

#endif