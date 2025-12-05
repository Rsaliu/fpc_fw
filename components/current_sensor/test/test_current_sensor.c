#include <current_sensor.h>
#include <unity.h>
#include <common_headers.h>
#include <stdio.h>
#include "esp_log.h"

current_sensor_t* current_sensor = NULL;
static const char* TAG = "CURRENT_SENSOR";

error_type_t dummy_read_current(void* context, float* current_value) {
    // Dummy function to simulate reading current
    // In a real scenario, this would read from the actual sensor hardware
    *current_value = 10.0f; // Example current value
    return SYSTEM_OK; // Simulate successful read
}

error_type_t current_sensor_overcurrent_monitor_func_callback(void* sensor, float threshold_current, overcurrent_comparator_callback_t callback, void* context){
    // Dummy function to simulate setting up overcurrent monitoring
    // In a real scenario, this would configure the sensor's comparator functionality
    ESP_LOGI(TAG, "Overcurrent monitoring set with threshold: %f", threshold_current);
    // using 16.0f as expected threshold because max_current is 20 and 80% of 20 is 16.0
    TEST_ASSERT_EQUAL_FLOAT(threshold_current, 16.0f); // Example expected threshold
    return SYSTEM_OK; // Simulate successful setup
}

// dummy void context for the current sensor callback
int current_sensor_dummy_value = 1;
void* current_sensor_dummy_context = NULL;
current_sensor_config_t current_sensor_config = {
    .id = 1,
    .context = &current_sensor_dummy_context, // Context for the callback, can be used to pass additional data
    .read_current = dummy_read_current, // Assuming a function pointer to read current is set later
    .make = "DummySensor",
    .max_current = 20,
    .overcurrent_callback = current_sensor_overcurrent_monitor_func_callback
};
void currentSensorSetUp(void) {
    // Set up code before each test

    current_sensor = current_sensor_create(&current_sensor_config);
}

void currentSensorTearDown(void) {
    // Clean up code after each test
    if (current_sensor != NULL) {
        current_sensor_destroy(&current_sensor);
    }
}

TEST_CASE("current_sensor_test", "test_current_sensor_create") {
    currentSensorSetUp();
    TEST_ASSERT_NOT_NULL(current_sensor);
    currentSensorTearDown();
}

TEST_CASE("current_sensor_test", "test_current_sensor_init") {
    currentSensorSetUp();
    error_type_t result = current_sensor_init(current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    currentSensorTearDown();
}

TEST_CASE("current_sensor_test", "test_current_sensor_deinit") {
    currentSensorSetUp();
    error_type_t result = current_sensor_init(current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = current_sensor_deinit(current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    currentSensorTearDown();
}

TEST_CASE("current_sensor_test", "test_current_sensor_destroy") {
    currentSensorSetUp();
    error_type_t result = current_sensor_init(current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = current_sensor_destroy(&current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(current_sensor); // Sensor should be NULL after destruction
}

TEST_CASE("current_sensor_test", "test_current_sensor_get_reading") {
    currentSensorSetUp();
    error_type_t result = current_sensor_init(current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    float current_value = 0.0f;
    result = current_sensor_get_current_in_amp(current_sensor, &current_value);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    ESP_LOGI(TAG,"Current reading: %f", current_value);
    TEST_ASSERT_EQUAL_FLOAT(current_value, 10.0f); // Check if the reading matches the dummy value
    currentSensorTearDown();
}
static void overcurrent_callback(overcurrent_queue_item_t item) {
    ESP_LOGI(TAG, "Overcurrent event detected!");
}
TEST_CASE("current_sensor_test", "test_current_sensor_monitor_overcurrent") {
    currentSensorSetUp();
    error_type_t result = current_sensor_init(current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Dummy callback function for overcurrent event

    result = current_sensor_monitor_overcurrent(current_sensor, overcurrent_callback, (void*)&current_sensor_dummy_value);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    currentSensorTearDown();
}