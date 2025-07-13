#ifndef __ACS712_CURRENT_SENSOR_H__
#define __ACS712_CURRENT_SENSOR_H__


#include <stdint.h>
#include <stdbool.h>
#include <esp_adc/adc_oneshot.h>
#include <common_headers.h>

typedef enum{
    ACS712_ADC_RES_10BIT,
    ACS712_ADC_RES_12BIT 
}acs712_adc_resolution_t;

typedef struct{
    adc_unit_t adc_unit_id;
    adc_oneshot_unit_handle_t adc_handle;
    adc_channel_t adc_channel;
    acs712_adc_resolution_t adc_res;
    adc_atten_t adc_atten;
    float vref;
    float sensitivity;
}acs712_config_t;


typedef struct acs712_sensor_t acs712_sensor_t;


acs712_sensor_t* acs712_create(acs712_config_t* config);
error_type_t acs712_sensor_init(acs712_sensor_t* sensor);
error_type_t acs712_sensor_deinit(acs712_sensor_t* sensor);
error_type_t acs712_destroy(acs712_sensor_t** sensor);
error_type_t acs712_calibrate_zero(acs712_sensor_t* sensor, float* zero_voltage);
error_type_t acs712_read_current(acs712_sensor_t* sensor, float* current);
error_type_t acs712_check_overcurrent(acs712_sensor_t* sensor, float threshold, bool* is_overcurrent);

#endif