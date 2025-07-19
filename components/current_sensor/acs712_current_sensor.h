#ifndef __ACS712_CURRENT_SENSOR_H__
#define __ACS712_CURRENT_SENSOR_H__


#include <stdint.h>
#include <stdbool.h>
#include <esp_adc/adc_oneshot.h>
#include <common_headers.h>

typedef error_type_t (*acs712_reading_callback_t)(void* context, int* adc_voltage);

typedef struct{
    acs712_reading_callback_t adc_reader;
    void** context; // Context for the callback, can be used to pass additional data
    int zero_voltage; // Zero voltage offset for the sensor
}acs712_config_t;


typedef struct acs712_sensor_t acs712_sensor_t;


acs712_sensor_t* acs712_create(acs712_config_t* config);
error_type_t acs712_sensor_init(acs712_sensor_t* sensor);
error_type_t acs712_sensor_deinit(acs712_sensor_t* sensor);
error_type_t acs712_destroy(acs712_sensor_t** sensor);
error_type_t acs712_read_current(const acs712_sensor_t* sensor, float* current);


#endif