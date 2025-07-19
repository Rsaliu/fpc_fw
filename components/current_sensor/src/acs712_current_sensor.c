#include "acs712_current_sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

static const char *TAG = "ACS712";
static const float ACS712_SENSITIVITY = 66; // ACS712_Sensitivity for ACS712 5A version in V/A 66mv/A

struct acs712_sensor_t {
    acs712_config_t* config;
    bool is_initialized;
};

static bool acs712_adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_bitwidth_t bitwidth, adc_cali_handle_t *out_handle) {
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "Calibration scheme: Line Fitting");
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = bitwidth,
    };
    ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
    if (ret == ESP_OK) {
        calibrated = true;
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "Line fitting not supported or eFuse not burnt, skip calibration");
    } else {
        ESP_LOGE(TAG, "Calibration failed: %s", esp_err_to_name(ret));
    }

    return calibrated;
}

acs712_sensor_t *acs712_create(acs712_config_t* config) {

    acs712_sensor_t *sensor = (acs712_sensor_t *)malloc(sizeof(acs712_sensor_t));
    if (!sensor) {
        ESP_LOGE(TAG, "Memory allocation failed");
        return NULL;
    }

    sensor->is_initialized = false;
    sensor->config = config;

    return sensor;
}

error_type_t acs712_sensor_init(acs712_sensor_t *sensor) {
    if (!sensor ) {
        ESP_LOGE(TAG, "NULL sensor pointer");
        return SYSTEM_NULL_PARAMETER;
    }
    if(!sensor->config->adc_reader) {
        ESP_LOGE(TAG, "ADC reader callback not set");
        return SYSTEM_NULL_PARAMETER;
    }
    if (sensor->is_initialized) {
        ESP_LOGE(TAG, "Sensor already initialized");
        return SYSTEM_INVALID_STATE;
    }
    sensor->is_initialized = true;
    ESP_LOGI(TAG, "ASC712 Sensor initialized");

    return SYSTEM_OK;
}

error_type_t acs712_sensor_deinit(acs712_sensor_t *sensor) {
    if (!sensor) {
        ESP_LOGE(TAG, "NULL sensor pointer");
        return SYSTEM_NULL_PARAMETER;
    }
    if (!sensor->is_initialized) {
        ESP_LOGE(TAG, "Sensor not initialized");
        return SYSTEM_INVALID_STATE;
    }
    sensor->is_initialized = false;
    ESP_LOGI(TAG, "Sensor deinitialized");
    return SYSTEM_OK;
}

error_type_t acs712_destroy(acs712_sensor_t **sensor) {
    if (!sensor || !*sensor) {
        ESP_LOGE(TAG, "NULL sensor pointer");
        return SYSTEM_NULL_PARAMETER;
    }

    if ((*sensor)->is_initialized) {
        error_type_t err = acs712_sensor_deinit(*sensor);
        if (err != SYSTEM_OK) {
            ESP_LOGE(TAG, "Deinit failed: %d", err);
            return err;
        }
    }

    free(*sensor);
    *sensor = NULL;
    ESP_LOGI(TAG, "Sensor destroyed");
    return SYSTEM_OK;
}

error_type_t acs712_read_current(const acs712_sensor_t* sensor, float* current){
    if (!sensor || !current) {
        ESP_LOGE(TAG, "NULL sensor or current pointer");
        return SYSTEM_NULL_PARAMETER;
    }
    if (!sensor->is_initialized) {
        ESP_LOGE(TAG, "Sensor not initialized");
        return SYSTEM_INVALID_STATE;
    }
    printf("Reading current for sensor with zero voltage: %d\n", sensor->config->zero_voltage);
    int voltage_value;
    error_type_t err = sensor->config->adc_reader(*sensor->config->context,&voltage_value);
    if (err != SYSTEM_OK) {
        ESP_LOGE(TAG, "ADC read failed");
        return SYSTEM_OPERATION_FAILED;
    }

    // Convert raw ADC value to current
    *current = (voltage_value - sensor->config->zero_voltage) / ACS712_SENSITIVITY; // Assuming 12-bit ADC resolution

    ESP_LOGI(TAG, "Current reading: %.2f A", *current);
    return SYSTEM_OK;
}