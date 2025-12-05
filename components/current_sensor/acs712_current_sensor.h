#ifndef __ACS712_CURRENT_SENSOR_H__
#define __ACS712_CURRENT_SENSOR_H__


#include <stdint.h>
#include <stdbool.h>
#include <esp_adc/adc_oneshot.h>
#include <common_headers.h>
#include <current_sensor_common.h>

typedef error_type_t (*acs712_reading_callback_t)(void* context, int* adc_voltage);

typedef error_type_t (*overcurrent_monitor_func_callback_t)(void* ads, const uint16_t high_threshold_value_in_millivolt, const uint16_t low_threshold_value_in_millivolt, overcurrent_comparator_callback_t comparator_callback, void* context);
typedef struct{
    acs712_reading_callback_t adc_reader;
    void** context; // Context for the callback, can be used to pass additional data
    int zero_voltage; // Zero voltage offset for the sensor
    overcurrent_monitor_func_callback_t overcurrent_monitor_func;   
}acs712_config_t;


typedef struct acs712_sensor_t acs712_sensor_t;


acs712_sensor_t* acs712_create(acs712_config_t* config);
error_type_t acs712_sensor_init(acs712_sensor_t* sensor);
error_type_t acs712_sensor_deinit(acs712_sensor_t* sensor);
error_type_t acs712_destroy(acs712_sensor_t** sensor);
error_type_t acs712_read_current(const acs712_sensor_t* sensor, float* current);
error_type_t acs712_monitor_overcurrent(const acs712_sensor_t* sensor, float threshold_current, overcurrent_comparator_callback_t callback, void* context);


#endif