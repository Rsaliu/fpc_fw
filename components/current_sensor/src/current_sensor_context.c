#include <current_sensor_context.h>
#include "esp_log.h"

error_type_t current_sensor_callback_ac712_adc_read(void* context, int* adc_voltage) {
    if (context == NULL || adc_voltage == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null context or adc_voltage pointer
    }
    adc_reader_t* adc_reader = (adc_reader_t*)context;
    error_type_t result;
    result = adc_reader_read(adc_reader, adc_voltage);
    if(result != SYSTEM_OK) {
        return result; // Handle error in reading ADC value
    }
    printf("ADC voltage read: %d\n", *adc_voltage);
    return SYSTEM_OK; // Return the ADC voltage read
}

error_type_t current_sensor_ac712_read_callback(void* context, float* current_value) {
    printf("current_sensor_ac712_read_callback called\n");
    acs712_sensor_t* sensor = (acs712_sensor_t*)context;
    if(sensor == NULL || current_value == NULL) {
        // print the pointer values for debugging
        printf("Sensor pointer: %p, Current value pointer: %p\n", sensor, current_value);
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    ESP_LOGI("current_sensor_ac712_read_callback", "current_sensor_ac712_read_callback called");
    error_type_t result = acs712_read_current(sensor, current_value);
    if(result != SYSTEM_OK) {
        return result; // Handle error in reading current
    }
    printf("Current value read: %f A\n", *current_value);
    return SYSTEM_OK; // Return the current value
}

error_type_t current_sensor_callback_ads1115_channel0_read(void* context, int* adc_voltage) {
    ads1115_t* sensor = (ads1115_t*)context;
    if(sensor == NULL || adc_voltage == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    int16_t raw_value = 0;
    ads1115_input_channel_t input_channel = ADS1115_CHANNEL_0; // Specify
    error_type_t result = ads1115_read(sensor, &raw_value, input_channel);
    if(result != SYSTEM_OK) {
        return result; // Handle error in reading current
    }
    *adc_voltage = raw_value; // Store the raw value in adc_voltage
    printf("ADC voltage read: %d\n", *adc_voltage);
    return SYSTEM_OK; // Return the current value
}

error_type_t current_sensor_callback_ads1115_channel1_read(void* context, int* adc_voltage) {
    ads1115_t* sensor = (ads1115_t*)context;
    if(sensor == NULL || adc_voltage == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    int16_t raw_value = 0;
    ads1115_input_channel_t input_channel = ADS1115_CHANNEL_1; // Specify
    error_type_t result = ads1115_read(sensor, &raw_value, input_channel);
    if(result != SYSTEM_OK) {
        return result; // Handle error in reading current
    }
    *adc_voltage = raw_value; // Store the raw value in adc_voltage
    printf("ADC voltage read: %d\n", *adc_voltage);
    return SYSTEM_OK; // Return the current value
}


error_type_t current_sensor_callback_ads1115_channel2_read(void* context, int* adc_voltage) {
    ads1115_t* sensor = (ads1115_t*)context;
    if(sensor == NULL || adc_voltage == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    int16_t raw_value = 0;
    ads1115_input_channel_t input_channel = ADS1115_CHANNEL_2; // Specify
    error_type_t result = ads1115_read(sensor, &raw_value, input_channel);
    if(result != SYSTEM_OK) {
        return result; // Handle error in reading current
    }
    *adc_voltage = raw_value; // Store the raw value in adc_voltage
    printf("ADC voltage read: %d\n", *adc_voltage);
    return SYSTEM_OK; // Return the current value
}

error_type_t current_sensor_callback_ads1115_channel3_read(void* context, int* adc_voltage) {
    ads1115_t* sensor = (ads1115_t*)context;
    if(sensor == NULL || adc_voltage == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    int16_t raw_value = 0;
    ads1115_input_channel_t input_channel = ADS1115_CHANNEL_3; // Specify
    error_type_t result = ads1115_read(sensor, &raw_value, input_channel);
    if(result != SYSTEM_OK) {
        return result; // Handle error in reading current
    }
    *adc_voltage = raw_value; // Store the raw value in adc_voltage
    printf("ADC voltage read: %d\n", *adc_voltage);
    return SYSTEM_OK; // Return the current value
}
