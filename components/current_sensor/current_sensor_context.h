#ifndef __CURRENT_SENSOR_FACTORIES_H__
#define __CURRENT_SENSOR_FACTORIES_H__
#include <common_headers.h>
#include <current_sensor.h>
#include <acs712_current_sensor.h>
#include <adc_reader.h>
#include <ads1115.h>


error_type_t current_sensor_callback_ac712_adc_read(void* context, int* adc_voltage);
error_type_t current_sensor_ac712_read_callback(void* context, float* current_value);
error_type_t current_sensor_callback_ads1115_channel0_read(void* context, int* adc_voltage);
error_type_t current_sensor_callback_ads1115_channel1_read(void* context, int* adc_voltage);
error_type_t current_sensor_callback_ads1115_channel2_read(void* context, int* adc_voltage);
error_type_t current_sensor_callback_ads1115_channel3_read(void* context, int* adc_voltage);

#endif // __CURRENT_SENSOR_FACTORIES_H__