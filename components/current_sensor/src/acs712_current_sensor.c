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

struct acs712_sensor_t {
    acs712_config_t config;
    float zero_voltage;
    adc_oneshot_unit_handle_t adc_handle;
    adc_cali_handle_t cali_handle;
    bool is_initialized;
};

static adc_bitwidth_t map_acs712_res_to_adc_bitwidth(acs712_adc_resolution_t res) {
    switch (res) {
    case ACS712_ADC_RES_10BIT:
        return ADC_BITWIDTH_10;
    case ACS712_ADC_RES_12BIT:
        return ADC_BITWIDTH_12;
    default:
        ESP_LOGW(TAG, "Invalid ADC resolution %d, defaulting to 12-bit", res);
        return ADC_BITWIDTH_12; 
    }
}

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

static error_type_t validate_config(const acs712_config_t *config) {
    if (!config) {
        ESP_LOGE(TAG, "NULL config pointer");
        return SYSTEM_NULL_PARAMETER;
    }

    if (config->adc_unit_id != ADC_UNIT_1 && config->adc_unit_id != ADC_UNIT_2) {
        ESP_LOGE(TAG, "Invalid ADC unit: %d", config->adc_unit_id);
        return SYSTEM_INVALID_PARAMETER;
    }

    if (config->adc_unit_id == ADC_UNIT_1) {
        if (config->adc_channel < ADC_CHANNEL_0 || config->adc_channel > ADC_CHANNEL_7) {
            ESP_LOGE(TAG, "Invalid ADC1 channel: %d", config->adc_channel);
            return SYSTEM_INVALID_PARAMETER;
        }
    } else { 
#if CONFIG_IDF_TARGET_ESP32
        if (config->adc_channel < ADC_CHANNEL_0 || config->adc_channel > ADC_CHANNEL_9) {
            ESP_LOGE(TAG, "Invalid ADC2 channel: %d", config->adc_channel);
            return SYSTEM_INVALID_PARAMETER;
        }
#else
        ESP_LOGE(TAG, "ADC2 not supported on this target");
        return SYSTEM_INVALID_PARAMETER;
#endif
    }

    if (!config->adc_handle) {
        ESP_LOGE(TAG, "NULL ADC handle provided");
        return SYSTEM_NULL_PARAMETER;
    }

    if (config->adc_atten != ADC_ATTEN_DB_0 && config->adc_atten != ADC_ATTEN_DB_2_5 &&
        config->adc_atten != ADC_ATTEN_DB_6 && config->adc_atten != ADC_ATTEN_DB_12) {
        ESP_LOGE(TAG, "Invalid ADC attenuation: %d", config->adc_atten);
        return SYSTEM_INVALID_PARAMETER;
    }

    if (config->vref < 2.25f || config->vref > 5.5f) {
        ESP_LOGE(TAG, "Invalid Vref: %.2f V", config->vref);
        return SYSTEM_INVALID_PARAMETER;
    }

    if (config->sensitivity < 0.032f || config->sensitivity > 0.068f) {
        ESP_LOGE(TAG, "Invalid sensitivity: %.3f mV/A", config->sensitivity);
        return SYSTEM_INVALID_PARAMETER;
    }

    if (config->adc_res != ACS712_ADC_RES_10BIT && config->adc_res != ACS712_ADC_RES_12BIT) {
        ESP_LOGE(TAG, "Invalid ADC resolution: %d", config->adc_res);
        return SYSTEM_INVALID_PARAMETER;
    }

    return SYSTEM_OK;
}

acs712_sensor_t *acs712_create(acs712_config_t *config) {
    if (!config || validate_config(config) != SYSTEM_OK) {
        ESP_LOGE(TAG, "Invalid config or NULL pointer");
        return NULL;
    }

    acs712_sensor_t *sensor = (acs712_sensor_t *)malloc(sizeof(acs712_sensor_t));
    if (!sensor) {
        ESP_LOGE(TAG, "Memory allocation failed");
        return NULL;
    }

    sensor->is_initialized = false;
    sensor->zero_voltage = config->vref / 2.0f;
    sensor->adc_handle = config->adc_handle;
    sensor->cali_handle = NULL;
    memcpy(&sensor->config, config, sizeof(acs712_config_t));

    ESP_LOGI(TAG, "Sensor created successfully with ADC%d handle", config->adc_unit_id + 1);
    return sensor;
}

error_type_t acs712_sensor_init(acs712_sensor_t *sensor) {
    if (!sensor) {
        ESP_LOGE(TAG, "NULL sensor pointer");
        return SYSTEM_NULL_PARAMETER;
    }
    if (sensor->is_initialized) {
        ESP_LOGE(TAG, "Sensor already initialized");
        return SYSTEM_INVALID_STATE;
    }
    if (validate_config(&sensor->config) != SYSTEM_OK) {
        ESP_LOGE(TAG, "Invalid configuration");
        return SYSTEM_INVALID_PARAMETER;
    }

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = sensor->config.adc_atten,
        .bitwidth = map_acs712_res_to_adc_bitwidth(sensor->config.adc_res),
    };

    esp_err_t err = adc_oneshot_config_channel(sensor->adc_handle, sensor->config.adc_channel, &chan_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ADC%d channel %d config failed: %s", sensor->config.adc_unit_id + 1, sensor->config.adc_channel, esp_err_to_name(err));
        return SYSTEM_OPERATION_FAILED;
    }

    bool calibrated = acs712_adc_calibration_init(sensor->config.adc_unit_id, sensor->config.adc_atten,
                                                 map_acs712_res_to_adc_bitwidth(sensor->config.adc_res),
                                                 &sensor->cali_handle);
    if (!calibrated) {
        ESP_LOGE(TAG, "ADC calibration failed");
        return SYSTEM_OPERATION_FAILED;
    }

    sensor->is_initialized = true;
    ESP_LOGI(TAG, "Sensor initialized on ADC%d channel %d, atten %d",
             sensor->config.adc_unit_id + 1, sensor->config.adc_channel, sensor->config.adc_atten);
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

    if (sensor->cali_handle) {
#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        if (adc_cali_delete_scheme_line_fitting(sensor->cali_handle) == ESP_OK) {
            ESP_LOGI(TAG, "Deregistered line fitting calibration");
        }
#endif
        sensor->cali_handle = NULL;
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

error_type_t acs712_calibrate_zero(acs712_sensor_t *sensor, float *zero_voltage) {
    if (!sensor || !zero_voltage || !sensor->is_initialized) {
        ESP_LOGE(TAG, "NULL pointer or sensor not initialized");
        return SYSTEM_NULL_PARAMETER;
    }

    const int samples = 100;
    uint32_t sum = 0;
    int raw;

    for (int i = 0; i < samples; i++) {
        esp_err_t err = adc_oneshot_read(sensor->adc_handle, sensor->config.adc_channel, &raw);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ADC read failed: %s", esp_err_to_name(err));
            return SYSTEM_OPERATION_FAILED;
        }
        sum += raw;
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    uint32_t avg_adc = sum / samples;
    int voltage_mv;
    esp_err_t err = adc_cali_raw_to_voltage(sensor->cali_handle, avg_adc, &voltage_mv);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ADC calibration failed: %s", esp_err_to_name(err));
        return SYSTEM_OPERATION_FAILED;
    }
    *zero_voltage = voltage_mv / 1000.0f; // mV to V
    sensor->zero_voltage = *zero_voltage;

    ESP_LOGI(TAG, "Calibrated zero voltage: %.2f V", *zero_voltage);
    return SYSTEM_OK;
}

error_type_t acs712_read_current(acs712_sensor_t *sensor, float *current) {
    if (!sensor || !current || !sensor->is_initialized) {
        ESP_LOGE(TAG, "NULL pointer or sensor not initialized");
        return SYSTEM_NULL_PARAMETER;
    }

    int raw;
    esp_err_t err = adc_oneshot_read(sensor->adc_handle, sensor->config.adc_channel, &raw);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ADC read failed: %s", esp_err_to_name(err));
        return SYSTEM_OPERATION_FAILED;
    }

    int voltage_mv;
    err = adc_cali_raw_to_voltage(sensor->cali_handle, raw, &voltage_mv);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ADC calibration failed: %s", esp_err_to_name(err));
        return SYSTEM_OPERATION_FAILED;
    }

    float v_iout = voltage_mv / 1000.0f; // mV to V
    *current = (v_iout - sensor->zero_voltage) / sensor->config.sensitivity;

    if (*current > 30.0f || *current < -30.0f) {
        ESP_LOGE(TAG, "Current out of range: %.2f A", *current);
        return SYSTEM_OUT_OF_RANGE;
    }

    ESP_LOGI(TAG, "Current: %.2f A", *current);
    return SYSTEM_OK;
}

error_type_t acs712_check_overcurrent(acs712_sensor_t *sensor, float threshold, bool *is_overcurrent) {
    if (!sensor || !is_overcurrent || !sensor->is_initialized) {
        ESP_LOGE(TAG, "NULL pointer or sensor not initialized");
        return SYSTEM_NULL_PARAMETER;
    }

    float current;
    error_type_t err = acs712_read_current(sensor, &current);
    if (err != SYSTEM_OK) {
        return err;
    }

    *is_overcurrent = (fabs(current) > threshold);
    if (*is_overcurrent) {
        ESP_LOGW(TAG, "Overcurrent detected: %.2f A > %.2f A", current, threshold);
    }
    return SYSTEM_OK;
}