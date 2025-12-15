#include <current_sensor.h>
#include <unity.h>
#include <common_headers.h>
#include <stdio.h>
#include "esp_log.h"

current_sensor_t* current_sensor = NULL;
static const char* TAG = "CURRENT_SENSOR";

typedef enum {
    CURRENT_SENSOR_TEST_CONFIG_BASIC_READING = 0,
    CURRENT_SENSOR_TEST_CONFIG_OVERCURRENT_MONITORING = 1,
    CURRENT_SENSOR_TEST_CONFIG_CONTINUOUS_MEASUREMENT = 2
} current_sensor_test_config_type_t;

error_type_t dummy_read_current(void* context, float* current_value) {
    // Dummy function to simulate reading current
    // In a real scenario, this would read from the actual sensor hardware
    *current_value = 10.0f; // Example current value
    return SYSTEM_OK; // Simulate successful read
}

error_type_t current_sensor_overcurrent_monitor_func_callback(void* sensor, float max_threshold_current, float min_threshold_current, overcurrent_comparator_callback_t callback, void* context){
    // Dummy function to simulate setting up overcurrent monitoring
    // In a real scenario, this would configure the sensor's comparator functionality
    ESP_LOGI(TAG, "Overcurrent monitoring set with threshold: %f", max_threshold_current);
    ESP_LOGI(TAG, "Undercurrent monitoring set with threshold: %f", min_threshold_current);
    TEST_ASSERT_EQUAL_FLOAT(max_threshold_current, 20.0f); // Example expected threshold
    return SYSTEM_OK; // Simulate successful setup
}

error_type_t current_sensor_continuous_read_func_callback(void* sensor,measurement_complete_callback_t callback, void* context){
    // Dummy function to simulate setting up overcurrent monitoring
    // In a real scenario, this would configure the sensor's comparator functionality
    return SYSTEM_OK; // Simulate successful setup
}

// dummy void context for the current sensor callback
int current_sensor_dummy_value = 1;
void* current_sensor_dummy_context = NULL;
current_sensor_config_t current_sensor_config_basic = {
    .id = 1,
    .context = &current_sensor_dummy_context, // Context for the callback, can be used to pass additional data
    .make = "DummySensor",
    .max_current = 20,
    .min_current = 0,
    .callback = (void*)dummy_read_current, // Assuming a function pointer to read current is set later
    .read_mode = CURRENT_SENSOR_READ_MODE_BASIC
};
current_sensor_config_t current_sensor_config_overcurrent = {
    .id = 1,
    .context = &current_sensor_dummy_context, // Context for the callback, can be used to pass additional data
    .make = "DummySensor",
    .max_current = 20,
    .min_current = 0,
    .callback = (void*)current_sensor_overcurrent_monitor_func_callback, // Assuming a function pointer to read current is set later
    .read_mode = CURRENT_SENSOR_READ_MODE_OVERCURRENT_MONITOR
};

current_sensor_config_t current_sensor_config_continuous = {
    .id = 1,
    .context = &current_sensor_dummy_context, // Context for the callback, can be used to pass additional data
    .make = "DummySensor",
    .max_current = 20,
    .min_current = 0,
    .callback = (void*)current_sensor_continuous_read_func_callback, // Assuming a function pointer to read current is set later
    .read_mode = CURRENT_SENSOR_READ_MODE_CONTINUOUS_MEASUREMENT
};


void currentSensorSetUp(current_sensor_test_config_type_t current_sensor_test_config_type) {
    // Set up code before each test
    switch (current_sensor_test_config_type){
        case CURRENT_SENSOR_TEST_CONFIG_BASIC_READING:
            // Set up for basic reading tests
            current_sensor = current_sensor_create(&current_sensor_config_basic);
            break;
        case CURRENT_SENSOR_TEST_CONFIG_OVERCURRENT_MONITORING:
            // Set up for overcurrent monitoring tests
            current_sensor = current_sensor_create(&current_sensor_config_overcurrent);
            break;
        case CURRENT_SENSOR_TEST_CONFIG_CONTINUOUS_MEASUREMENT:
            // Set up for continuous measurement tests
            current_sensor = current_sensor_create(&current_sensor_config_continuous);
            break;
        default:
            // Default setup
            current_sensor = current_sensor_create(&current_sensor_config_basic);
            break;
    }
}

void currentSensorTearDown(void) {
    // Clean up code after each test
    if (current_sensor != NULL) {
        current_sensor_destroy(&current_sensor);
    }
}

TEST_CASE("current_sensor_test", "test_current_sensor_create") {
    currentSensorSetUp(CURRENT_SENSOR_TEST_CONFIG_BASIC_READING);
    TEST_ASSERT_NOT_NULL(current_sensor);
    currentSensorTearDown();
}

TEST_CASE("current_sensor_test", "test_current_sensor_init") {
    currentSensorSetUp(CURRENT_SENSOR_TEST_CONFIG_BASIC_READING);
    error_type_t result = current_sensor_init(current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    currentSensorTearDown();
}

TEST_CASE("current_sensor_test", "test_current_sensor_deinit") {
    currentSensorSetUp(CURRENT_SENSOR_TEST_CONFIG_BASIC_READING);
    error_type_t result = current_sensor_init(current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = current_sensor_deinit(current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    currentSensorTearDown();
}

TEST_CASE("current_sensor_test", "test_current_sensor_destroy") {
    currentSensorSetUp(CURRENT_SENSOR_TEST_CONFIG_BASIC_READING);
    error_type_t result = current_sensor_init(current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = current_sensor_destroy(&current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(current_sensor); // Sensor should be NULL after destruction
}

TEST_CASE("current_sensor_test", "test_current_sensor_get_reading") {
    currentSensorSetUp(CURRENT_SENSOR_TEST_CONFIG_BASIC_READING);
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
    currentSensorSetUp(CURRENT_SENSOR_TEST_CONFIG_OVERCURRENT_MONITORING);
    error_type_t result = current_sensor_init(current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Dummy callback function for overcurrent event

    result = current_sensor_monitor_overcurrent(current_sensor, overcurrent_callback, (void*)&current_sensor_dummy_value);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    currentSensorTearDown();
}

static void continuous_read_callback(measurement_item_t item) {
    ESP_LOGI(TAG, "continuous read event detected!");
}

TEST_CASE("current_sensor_test", "test_current_sensor_continuous_read") {
    currentSensorSetUp(CURRENT_SENSOR_TEST_CONFIG_CONTINUOUS_MEASUREMENT);
    error_type_t result = current_sensor_init(current_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Dummy callback function for overcurrent event

    result = current_sensor_continuous_read(current_sensor, continuous_read_callback, (void*)&current_sensor_dummy_value);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    currentSensorTearDown();
}