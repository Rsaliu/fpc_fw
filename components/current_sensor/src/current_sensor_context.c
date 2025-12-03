#include <current_sensor_context.h>
#include "esp_log.h"
static const char* TAG = "CURRENT_SENSOR_CONTEXT";
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
    ESP_LOGI(TAG, "ADC voltage read: %d", *adc_voltage);
    return SYSTEM_OK; // Return the ADC voltage read
}

error_type_t current_sensor_ac712_read_callback(void* context, float* current_value) {
    ESP_LOGI(TAG, "current_sensor_ac712_read_callback called");
    acs712_sensor_t* sensor = (acs712_sensor_t*)context;
    if(sensor == NULL || current_value == NULL) {
        // print the pointer values for debugging
        ESP_LOGI(TAG, "Sensor pointer: %p, Current value pointer: %p", sensor, current_value);
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    ESP_LOGI("current_sensor_ac712_read_callback", "current_sensor_ac712_read_callback called");
    error_type_t result = acs712_read_current(sensor, current_value);
    if(result != SYSTEM_OK) {
        return result; // Handle error in reading current
    }
    ESP_LOGI(TAG, "Current value read: %f A\n", *current_value);
    return SYSTEM_OK; // Return the current value
}

error_type_t current_sensor_callback_ads1115_channel0_read(void* context, int* adc_voltage) {
    ads1115_t* sensor = (ads1115_t*)context;
    if(sensor == NULL || adc_voltage == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    int16_t raw_value = 0;
    ads1115_input_channel_t input_channel = ADS1115_CHANNEL_0; // Specify
    error_type_t result = ads1115_read_one_shot_with_channel(sensor, &raw_value, input_channel);
    if(result != SYSTEM_OK) {
        return result; // Handle error in reading current
    }
    *adc_voltage = raw_value; // Store the raw value in adc_voltage
    ESP_LOGI(TAG, "ADC voltage read: %d", *adc_voltage);
    return SYSTEM_OK; // Return the current value
}

error_type_t current_sensor_callback_ads1115_channel1_read(void* context, int* adc_voltage) {
    ads1115_t* sensor = (ads1115_t*)context;
    if(sensor == NULL || adc_voltage == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    int16_t raw_value = 0;
    ads1115_input_channel_t input_channel = ADS1115_CHANNEL_1; // Specify
    error_type_t result = ads1115_read_one_shot_with_channel(sensor, &raw_value, input_channel);
    if(result != SYSTEM_OK) {
        return result; // Handle error in reading current
    }
    *adc_voltage = raw_value; // Store the raw value in adc_voltage
    ESP_LOGI(TAG, "ADC voltage read: %d\n", *adc_voltage);
    return SYSTEM_OK; // Return the current value
}


error_type_t current_sensor_callback_ads1115_channel2_read(void* context, int* adc_voltage) {
    ads1115_t* sensor = (ads1115_t*)context;
    if(sensor == NULL || adc_voltage == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    int16_t raw_value = 0;
    ads1115_input_channel_t input_channel = ADS1115_CHANNEL_2; // Specify
    error_type_t result = ads1115_read_one_shot_with_channel(sensor, &raw_value, input_channel);
    if(result != SYSTEM_OK) {
        return result; // Handle error in reading current
    }
    *adc_voltage = raw_value; // Store the raw value in adc_voltage
    ESP_LOGI(TAG, "ADC voltage read: %d\n", *adc_voltage);
    return SYSTEM_OK; // Return the current value
}

error_type_t current_sensor_callback_ads1115_channel3_read(void* context, int* adc_voltage) {
    ads1115_t* sensor = (ads1115_t*)context;
    if(sensor == NULL || adc_voltage == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor
    }
    int16_t raw_value = 0;
    ads1115_input_channel_t input_channel = ADS1115_CHANNEL_3; // Specify
    error_type_t result = ads1115_read_one_shot_with_channel(sensor, &raw_value, input_channel);
    if(result != SYSTEM_OK) {
        return result; // Handle error in reading current
    }
    *adc_voltage = raw_value; // Store the raw value in adc_voltage
    ESP_LOGI(TAG, "ADC voltage read: %d\n", *adc_voltage);
    return SYSTEM_OK; // Return the current value
}
