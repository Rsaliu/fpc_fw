#include <acs712_current_sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <esp_adc/adc_oneshot.h>
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
        .adc_atten = ADC_ATTEN_DB_11,
        .vref = 5.0f,
        .sensitivity = 0.066f};
    acs712_sensor = acs712_create(&config);
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

// TEST_CASE("acs712_test", "test_acs712_create") {
//     acs712SetUp();
//     TEST_ASSERT_NOT_NULL(acs712_sensor);
//     TEST_ASSERT_FALSE(acs712_sensor->is_initialized);
//     TEST_ASSERT_EQUAL_FLOAT(2.5f, acs712_sensor->zero_voltage);
//     TEST_ASSERT_EQUAL_PTR(adc_handle, acs712_sensor->adc_handle);
//     TEST_ASSERT_NULL(acs712_sensor->cali_handle);
//     acs712TearDown();
// }

// TEST_CASE("acs712_test", "test_acs712_init") {
//     acs712SetUp();
//     error_type_t result = acs712_sensor_init(acs712_sensor);
//     TEST_ASSERT_EQUAL(SYSTEM_OK, result);
//     TEST_ASSERT_TRUE(acs712_sensor->is_initialized);
//     TEST_ASSERT_EQUAL_PTR(adc_handle, acs712_sensor->adc_handle);

//     TEST_ASSERT_NOT_NULL(acs712_sensor->cali_handle);
//     acs712TearDown();
// }

// TEST_CASE("acs712_test", "test_acs712_init_invalid_config") {
//     acs712SetUp();
//     acs712_sensor->config.adc_channel = ADC_CHANNEL_8;
//     error_type_t result = acs712_sensor_init(acs712_sensor);
//     TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result);
//     TEST_ASSERT_FALSE(acs712_sensor->is_initialized);
//     acs712_sensor->config.adc_channel = ADC_CHANNEL_6;
//     acs712TearDown();
// }

TEST_CASE("acs712_test", "test_acs712_create")
{
    setUp();
    TEST_ASSERT_NOT_NULL(acs712_sensor);
    TEST_ASSERT_FALSE(acs712_sensor->is_initialized);
    TEST_ASSERT_EQUAL_FLOAT(1.25f, acs712_sensor->zero_voltage); // vref/2 = 2.5/2
    TEST_ASSERT_EQUAL_PTR(adc_handle, acs712_sensor->adc_handle);
    TEST_ASSERT_NULL(acs712_sensor->cali_handle);
    tearDown();
}

TEST_CASE("acs712_test", "test_acs712_init_adc1")
{
    setUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_TRUE(acs712_sensor->is_initialized);
    TEST_ASSERT_EQUAL_PTR(adc_handle, acs712_sensor->adc_handle);
    TEST_ASSERT_NOT_NULL(acs712_sensor->cali_handle);
    tearDown();
}

#if CONFIG_IDF_TARGET_ESP32
TEST_CASE("acs712_test", "test_acs712_init_adc2")
{
    setUp();
    adc_oneshot_del_unit(adc_handle); // Clean up ADC1 handle
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
    acs712_sensor->config.adc_unit_id = ADC_UNIT_2;
    acs712_sensor->config.adc_handle = adc_handle;
    acs712_sensor->config.adc_channel = ADC_CHANNEL_0; // GPIO0 for ESP32 ADC2
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_TRUE(acs712_sensor->is_initialized);
    TEST_ASSERT_EQUAL_PTR(adc_handle, acs712_sensor->adc_handle);
    TEST_ASSERT_NOT_NULL(acs712_sensor->cali_handle);
    tearDown();
}
#endif

TEST_CASE("acs712_test", "test_acs712_init_invalid_config")
{
    setUp();
    // Invalid ADC unit
    acs712_sensor->config.adc_unit_id = -1;
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result);
    TEST_ASSERT_FALSE(acs712_sensor->is_initialized);
    // Invalid ADC1 channel
    acs712_sensor->config.adc_unit_id = ADC_UNIT_1;
    acs712_sensor->config.adc_channel = ADC_CHANNEL_8;
    result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result);
    TEST_ASSERT_FALSE(acs712_sensor->is_initialized);
    // NULL ADC handle
    acs712_sensor->config.adc_channel = ADC_CHANNEL_6;
    acs712_sensor->config.adc_handle = NULL;
    result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result);
    TEST_ASSERT_FALSE(acs712_sensor->is_initialized);
    tearDown();
}

#if CONFIG_IDF_TARGET_ESP32
TEST_CASE("acs712_test", "test_acs712_init_invalid_adc2_channel")
{
    setUp();
    adc_oneshot_del_unit(adc_handle);
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
    acs712_sensor->config.adc_unit_id = ADC_UNIT_2;
    acs712_sensor->config.adc_handle = adc_handle;
    acs712_sensor->config.adc_channel = ADC_CHANNEL_10; // Invalid for ADC2
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result);
    TEST_ASSERT_FALSE(acs712_sensor->is_initialized);
    tearDown();
}
#else
TEST_CASE("acs712_test", "test_acs712_init_invalid_adc2")
{
    setUp();
    adc_oneshot_del_unit(adc_handle);
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
    acs712_sensor->config.adc_unit_id = ADC_UNIT_2;
    acs712_sensor->config.adc_handle = adc_handle;
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result);
    TEST_ASSERT_FALSE(acs712_sensor->is_initialized);
    tearDown();
}
#endif

TEST_CASE("acs712_test", "test_acs712_calibrate_zero")
{
    acs712SetUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    float zero_voltage;
    result = acs712_calibrate_zero(acs712_sensor, &zero_voltage);
    printf("zero voltage is: %f\n", zero_voltage);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_TRUE(zero_voltage >= 0.0f && zero_voltage <= 2.5f); // Range with divider
    TEST_ASSERT_EQUAL_FLOAT(zero_voltage, acs712_sensor->zero_voltage);
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
    acs712tearDown();
}

TEST_CASE("acs712_test", "test_acs712_deinit")
{
    asc712setUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = acs712_sensor_deinit(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_FALSE(acs712_sensor->is_initialized);
    TEST_ASSERT_NULL(acs712_sensor->cali_handle);
    TEST_ASSERT_EQUAL_PTR(adc_handle, acs712_sensor->adc_handle); // External handle persists
    asc712tearDown();
}

TEST_CASE("acs712_test", "test_acs712_destroy")
{
    acs712setUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = acs712_destroy(&acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(acs712_sensor);
    float current;
    result = acs712_read_current(acs712_sensor, &current);
    TEST_ASSERT_EQUAL(SYSTEM_NULL_PARAMETER, result);
    acs712tearDown();
}
```
