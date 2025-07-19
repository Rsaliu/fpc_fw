#include <acs712_current_sensor.h>
#include <esp_log.h>
#include <stdlib.h>
#include <unity.h>

#define ACS712_ZERO_VOLTAGE_DEFAULT 2500 // Default zero voltage for ACS712 converted from 5V to 3.3V range
#define ACS712_SENSITIVITY 66 // ACS712 Sensitivity for ACS712 5
acs712_sensor_t* acs712_sensor = NULL;

error_type_t get_adc_value(void* context, int* adc_voltage) {
    // Dummy function to simulate ADC reading
    // In a real scenario, this would read from the ADC hardware
    *adc_voltage = 2048; // Example ADC value for 0V (midpoint for a 12-bit ADC)

    return SYSTEM_OK; // Example raw ADC value
}
// dunmmy void context for the ADC reader callback
int dummy_value = 1;
void* dummy_context = &dummy_value;
acs712_config_t acs712_config = {
    .adc_reader = get_adc_value,
    .context = &dummy_context, // Context for the callback, can be used to pass additional data
    .zero_voltage = 2500, // Zero voltage offset for the sensor
}; // Initialize with a NULL ADC reader
// Set up function for ACS712 sensor tests
void acs712SensorSetUp(void) {
    // Set up code before each test

    acs712_sensor = acs712_create(&acs712_config);
}

void acs712SensorTearDown(void) {
    // Clean up code after each test
    if (acs712_sensor != NULL) {
        acs712_destroy(&acs712_sensor);
    }
}



TEST_CASE("acs712_current_sensor_test", "test_acs712_create") {
    acs712SensorSetUp();
    TEST_ASSERT_NOT_NULL(acs712_sensor);
    acs712SensorTearDown();
}

TEST_CASE("acs712_current_sensor_test", "test_acs712_init") {
    acs712SensorSetUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    acs712SensorTearDown();
}

TEST_CASE("acs712_current_sensor_test", "test_acs712_deinit") {
    acs712SensorSetUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = acs712_sensor_deinit(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    acs712SensorTearDown();
}

TEST_CASE("acs712_current_sensor_test", "test_acs712_destroy") {
    acs712SensorSetUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = acs712_destroy(&acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(acs712_sensor); // Sensor should be NULL after destruction
}

TEST_CASE("acs712_current_sensor_test", "test_acs712_read_current") {
    acs712SensorSetUp();
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    float current_value = 0.0f;
    result = acs712_read_current(acs712_sensor, &current_value);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    printf("Current reading: %f\n", current_value);
    float expected_current = (float)(2048 - ACS712_ZERO_VOLTAGE_DEFAULT) / ACS712_SENSITIVITY; // Assuming 12-bit ADC resolution
    TEST_ASSERT_EQUAL_FLOAT(current_value, expected_current); // Assuming 12-bit ADC resolution
    acs712SensorTearDown();
}