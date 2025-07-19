#include <adc_reader.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_log.h>
#include <stdlib.h>

static const char *TAG = "ADC_READER";

struct adc_reader_t {
    adc_reader_config_t* config; // Configuration for the ADC reader
    adc_oneshot_unit_handle_t adc_handle; // Handle for the ADC unit
    adc_cali_handle_t adc_cali_handle;
    bool is_calibrated; // Flag to check if the ADC is calibrated
    bool is_initialized; // Flag to check if the ADC reader is initialized
};
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void adc_calibration_deinit(adc_cali_handle_t handle);

adc_reader_t* adc_reader_create(const adc_reader_config_t* config){
    adc_reader_t *adc_reader = (adc_reader_t *)malloc(sizeof(adc_reader_t));
    if (adc_reader == NULL) {
        return NULL; // Handle memory allocation failure
    }

    adc_reader->config = config;
    adc_reader->is_initialized = false; // Initially not initialized
    adc_reader->adc_handle = NULL; // ADC handle is NULL until initialized

    return adc_reader;
}
error_type_t adc_reader_init(adc_reader_t* adc_reader){
    if (adc_reader == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null ADC reader
    }
    if (adc_reader->is_initialized) {
        return SYSTEM_INVALID_STATE; // ADC reader is already initialized
    }

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = adc_reader->config->adc_unit_id,
        //.bitwidth = adc_reader->config->adc_bitwidth,
    };

    esp_err_t err = adc_oneshot_new_unit(&init_config, &adc_reader->adc_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ADC initialization failed: %s", esp_err_to_name(err));
        return SYSTEM_OPERATION_FAILED;
    }

    adc_oneshot_chan_cfg_t config = {
        .atten = adc_reader->config->adc_atten, // Set the attenuation
        .bitwidth = adc_reader->config->adc_bitwidth, // Set the bit width
    };
    adc_oneshot_config_channel(adc_reader->adc_handle, adc_reader->config->adc_channel, &config);
    adc_reader->is_calibrated = adc_calibration_init(adc_reader->config->adc_unit_id,
                                             adc_reader->config->adc_channel,
                                              adc_reader->config->adc_atten, &adc_reader->adc_cali_handle);
    adc_reader->is_initialized = true; // Set the initialized flag
    ESP_LOGI(TAG, "ADC reader initialized successfully");
    return SYSTEM_OK;
}
error_type_t adc_reader_read(const adc_reader_t* adc_reader, int *raw_value){
    if (adc_reader == NULL || raw_value == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null ADC reader or raw value pointer
    }
    if (!adc_reader->is_initialized) {
        return SYSTEM_INVALID_STATE; // ADC reader is not initialized
    }

    esp_err_t err = adc_oneshot_read(adc_reader->adc_handle, adc_reader->config->adc_channel, raw_value);
    if (err != SYSTEM_OK){
        ESP_LOGE(TAG, "ADC read failed: %s", esp_err_to_name(err));
        return SYSTEM_OPERATION_FAILED;
    }
    if (adc_reader->is_calibrated) {
        int calibrated_value = 0;
        err = adc_cali_raw_to_voltage(adc_reader->adc_cali_handle, *raw_value, &calibrated_value);
        if (err != SYSTEM_OK) {
            ESP_LOGE(TAG, "ADC calibration failed: %s", esp_err_to_name(err));
            return SYSTEM_OPERATION_FAILED;
        }
        *raw_value = calibrated_value; // Use the calibrated value
    }
    ESP_LOGI(TAG, "ADC read value: %d", *raw_value);
    return SYSTEM_OK;
}
error_type_t adc_reader_deinit(adc_reader_t* adc_reader){
    if (adc_reader == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null ADC reader
    }
    if (!adc_reader->is_initialized) {
        return SYSTEM_INVALID_STATE; // ADC reader is not initialized
    }

    esp_err_t err = adc_oneshot_del_unit(adc_reader->adc_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ADC deinitialization failed: %s", esp_err_to_name(err));
        return SYSTEM_OPERATION_FAILED;
    }

    if (adc_reader->is_calibrated) {
        adc_calibration_deinit(adc_reader->adc_cali_handle); // Deinitialize the calibration handle
    }

    adc_reader->is_initialized = false; // Reset the initialized flag
    adc_reader->adc_handle = NULL; // Reset the ADC handle
    ESP_LOGI(TAG, "ADC reader deinitialized successfully");
    return SYSTEM_OK;
}
error_type_t adc_reader_destroy(adc_reader_t** adc_reader){
    if (adc_reader == NULL || *adc_reader == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null ADC reader pointer
    }

    if ((*adc_reader)->is_initialized) {
        error_type_t err = adc_reader_deinit(*adc_reader);
        if (err != SYSTEM_OK) {
            ESP_LOGE(TAG, "Deinitialization failed: %d", err);
            return err; // Handle error in deinitialization
        }
    }

    free(*adc_reader); // Free the allocated memory for the ADC reader
    *adc_reader = NULL; // Set the pointer to NULL after freeing
    ESP_LOGI(TAG, "ADC reader destroyed successfully");
    return SYSTEM_OK;
}


static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static void adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}
