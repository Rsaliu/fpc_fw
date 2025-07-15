#include <acs712_current_sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>   
#include <esp_adc/adc_cali_scheme.h>
#include <esp_log.h>
#include "unity.h"

static const char* TAG = "TEST_ACS712";

static acs712_sensor_t* acs712_sensor = NULL;
static adc_oneshot_unit_handle_t adc_handle = NULL;

void acs712SetUp(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    acs712_config_t config = {
        .adc_unit_id = ADC_UNIT_1,
        .adc_handle = adc_handle,
        .adc_channel = ADC_CHANNEL_6,
        .adc_res = ACS712_ADC_RES_12BIT,
        .adc_atten = ADC_ATTEN_DB_12,
        .vref = 2.5f,
        .sensitivity = 0.066f
    };
    acs712_sensor = acs712_create(&config);
    if (acs712_sensor == NULL) {
        ESP_LOGE(TAG, "acs712_create failed");
    }
    TEST_ASSERT_NOT_NULL_MESSAGE(acs712_sensor, "Failed to create ACS712 sensor");
}

void acs712TearDown(void)
{

    if (acs712_sensor != NULL)
    {
        acs712_destroy(&acs712_sensor);
    }
    if (adc_handle != NULL)
    {
        adc_oneshot_del_unit(adc_handle);
        adc_handle = NULL;
    }
}


TEST_CASE("acs712_test", "test_acs712_create")
{
    acs712SetUp();
    TEST_ASSERT_NOT_NULL(acs712_sensor); 
    ESP_LOGI(TAG, "Initial zero voltage: %.2f V", acs712_sensor->zero_voltage);
    acs712TearDown();
}

TEST_CASE("acs712_test", "test_acs712_init_adc1")
{
    acs712SetUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_TRUE(acs712_sensor->is_initialized);
    ESP_LOGI(TAG, "Sensor initialized on ADC1 channel %d", acs712_sensor->config.adc_channel);
    acs712TearDown();
}

#if CONFIG_IDF_TARGET_ESP32
TEST_CASE("acs712_test", "test_acs712_init_adc2") {
    acs712SetUp();
    adc_oneshot_del_unit(adc_handle); 
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
    acs712_config_t config = { 
        .adc_unit_id = ADC_UNIT_2,
        .adc_handle = adc_handle,
        .adc_channel = ADC_CHANNEL_0, 
        .adc_res = ACS712_ADC_RES_12BIT,
        .adc_atten = ADC_ATTEN_DB_12,
        .vref = 2.5f,
        .sensitivity = 0.066f
    };
    acs712_destroy(&acs712_sensor); 
    acs712_sensor = acs712_create(&config);
    TEST_ASSERT_NOT_NULL(acs712_sensor);
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    acs712TearDown();
}
#endif

TEST_CASE("acs712_test", "test_acs712_init_invalid_config") {
    acs712SetUp();
    acs712_config_t invalid_config = acs712_sensor->config; // Copy config
    invalid_config.adc_unit_id = -1;
    acs712_destroy(&acs712_sensor); 
    acs712_sensor = acs712_create(&invalid_config);
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result);
    acs712TearDown();

    acs712SetUp();
    invalid_config.adc_unit_id = ADC_UNIT_1;
    invalid_config.adc_channel = ADC_CHANNEL_8;
    acs712_destroy(&acs712_sensor);
    acs712_sensor = acs712_create(&invalid_config);
    result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result);
    acs712TearDown();

    acs712SetUp();
    invalid_config.adc_channel = ADC_CHANNEL_6;
    invalid_config.adc_handle = NULL;
    acs712_destroy(&acs712_sensor);
    acs712_sensor = acs712_create(&invalid_config);
    result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result);
    acs712TearDown();
}

#if CONFIG_IDF_TARGET_ESP32
TEST_CASE("acs712_test", "test_acs712_init_invalid_adc2_channel") {
    acs712SetUp();
    adc_oneshot_del_unit(adc_handle);
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
    acs712_config_t invalid_config = {
        .adc_unit_id = ADC_UNIT_2,
        .adc_handle = adc_handle,
        .adc_channel = ADC_CHANNEL_9, 
        .adc_res = ACS712_ADC_RES_12BIT,
        .adc_atten = ADC_ATTEN_DB_12,
        .vref = 2.5f,
        .sensitivity = 0.066f
    };
    acs712_destroy(&acs712_sensor);
    acs712_sensor = acs712_create(&invalid_config);
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result);
    acs712TearDown();
}
#else
TEST_CASE("acs712_test", "test_acs712_init_invalid_adc2") {
    acs712SetUp();
    adc_oneshot_del_unit(adc_handle);
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
    acs712_config_t invalid_config = {
        .adc_unit_id = ADC_UNIT_2,
        .adc_handle = adc_handle,
        .adc_channel = ADC_CHANNEL_0,
        .adc_res = ACS712_ADC_RES_12BIT,
        .adc_atten = ADC_ATTEN_DB_12,
       .vref = 2.5f,
        .sensitivity = 0.066f
    };
    acs712_destroy(&acs712_sensor);
    acs712_sensor = acs712_create(&invalid_config);
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result);
    acs712TearDown();
}
#endif



TEST_CASE("acs712_test", "test_acs712_calibrate_zero")
{
    acs712SetUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    float zero_voltage;
    result = acs712_calibrate_zero(acs712_sensor, &zero_voltage);
    printf("zero voltage is: %.2f V\n", zero_voltage);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_FLOAT_WITHIN(0.2f, 1.666f, zero_voltage);
    acs712TearDown();
}

TEST_CASE("acs712_test", "test_acs712_measure_current_with_load") {
    acs712SetUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    float zero_voltage;
    result = acs712_calibrate_zero(acs712_sensor, &zero_voltage);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    ESP_LOGI(TAG, "Calibrated zero voltage: %.2f V", zero_voltage);

    float current;
    result = acs712_read_current(acs712_sensor, &current);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    ESP_LOGI(TAG, "Measured current: %.2f mA", current * 1000); // Convert to mA
    TEST_ASSERT_FLOAT_WITHIN(0.2f, 1.0f, current)
    acs712TearDown();
}

TEST_CASE("acs712_test", "test_acs712_check_overcurrent")
{
    acs712SetUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    bool is_overcurrent;
    result = acs712_check_overcurrent(acs712_sensor, 30.0f, &is_overcurrent);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    acs712TearDown();
}

TEST_CASE("acs712_test", "test_acs712_deinit") {
    acs712SetUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    bool is_overcurrent;
    result = acs712_check_overcurrent(acs712_sensor, 30.0f, &is_overcurrent);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    acs712TearDown();
}

TEST_CASE("acs712_test", "test_acs712_deinit") {
    acs712SetUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = acs712_sensor_deinit(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_FALSE(acs712_sensor->is_initialized); 
    acs712TearDown();
}

TEST_CASE("acs712_test", "test_acs712_destroy") {
    acs712SetUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = acs712_destroy(&acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(acs712_sensor);
    float current;
    result = acs712_read_current(acs712_sensor, &current);
    TEST_ASSERT_EQUAL(SYSTEM_NULL_PARAMETER, result);
    acs712TearDown();
}

