#include <acs712_current_sensor.h>
#include <esp_log.h>
#include <stdlib.h>
#include <unity.h>

#define ACS712_ZERO_VOLTAGE_DEFAULT 2500 // Default zero voltage for ACS712 converted from 5V to 3.3V range
#define ACS712_SENSITIVITY 66 // ACS712 Sensitivity for ACS712 5

typedef enum{
    ACS712_TEST_CONFIG_ORDINARY,
    ACS712_TEST_CONFIG_OVERCURRENT_MONITOR,
    ACS712_TEST_CONFIG_CONTINUOUS_MEASUREMENT
} acs712_test_config_type_t;

acs712_sensor_t* acs712_sensor = NULL;
const double OVERCURRENT_THRESHOLD_CURRENT = 10; // Example threshold current in Amperes
const double UNDERCURRENT_THRESHOLD_CURRENT = 0; // Example threshold current in Amperes

 void overcurrent_comparator_callback (overcurrent_queue_item_t item){
    // Dummy callback function for overcurrent monitoring
    printf("Overcurrent event detected! Timestamp: %ld, Channel: %d\n", item.timestamp, item.channel);
 }

 void measurement_complete_callback_func (measurement_item_t item){
    // Dummy callback function for measurement complete
    printf("Measurement complete event detected! Channel: %d\n", item.channel);
 }

error_type_t overcurrent_monitor_func_callback (void* ads, const uint16_t high_threshold_value_in_millivolt, const uint16_t low_threshold_value_in_millivolt, overcurrent_comparator_callback_t comparator_callback, void* context)
{
    // Dummy function to simulate setting up overcurrent monitoring
    // In a real scenario, this would configure the ADS1115 comparator
    printf("Setting up overcurrent monitoring with high threshold: %d mV, low threshold: %d mV\n", high_threshold_value_in_millivolt, low_threshold_value_in_millivolt);
    TEST_ASSERT_EQUAL(high_threshold_value_in_millivolt, 3160); // Example expected high threshold
    TEST_ASSERT_EQUAL(low_threshold_value_in_millivolt, 1840); // Example expected low threshold
    return SYSTEM_OK;
}

error_type_t measurement_complete_monitor_func_callback (void* ads, measurement_complete_callback_t measurement_callback, void* context){
    return SYSTEM_OK;
}

error_type_t get_adc_value(void* context, int* adc_voltage) {
    // Dummy function to simulate ADC reading
    // In a real scenario, this would read from the ADC hardware
    *adc_voltage = 2048; // Example ADC value for 0V (midpoint for a 12-bit ADC)

    return SYSTEM_OK; // Example raw ADC value
}
// dunmmy void context for the ADC reader callback
int dummy_value = 1;
void* dummy_context = &dummy_value;

acs712_config_t acs712_config_ordinary = {
    .context = &dummy_context, // Context for the callback, can be used to pass additional data
    .zero_voltage = 2500, // Zero voltage offset for the sensor
    .callback_func = (void*)get_adc_value,
    .read_mode = ACS712_READ_MODE_BASIC
}; // Initialize with a NULL ADC reader

acs712_config_t acs712_config_overcurrent_monitor = {
    .context = &dummy_context, // Context for the callback, can be used to pass additional data
    .zero_voltage = 2500, // Zero voltage offset for the sensor
    .callback_func = (void*)overcurrent_monitor_func_callback,
    .read_mode = ACS712_READ_MODE_OVERCURRENT_MONITOR
}; // Initialize with a NULL ADC reader

acs712_config_t acs712_config_continuous_measurement = {
    .context = &dummy_context, // Context for the callback, can be used to pass additional data
    .zero_voltage = 2500, // Zero voltage offset for the sensor
    .callback_func = (void*)measurement_complete_monitor_func_callback,
    .read_mode = ACS712_READ_MODE_CONTINUOUS_MEASUREMENT
}; // Initialize with a NULL ADC reader

// Set up function for ACS712 sensor tests
void acs712SensorSetUp(acs712_test_config_type_t config_type) {
    // Set up code before each test
    switch(config_type){
        case ACS712_TEST_CONFIG_ORDINARY:
            acs712_sensor = acs712_create(&acs712_config_ordinary);
            break;
        case ACS712_TEST_CONFIG_OVERCURRENT_MONITOR:
            acs712_sensor = acs712_create(&acs712_config_overcurrent_monitor);
            break;
        case ACS712_TEST_CONFIG_CONTINUOUS_MEASUREMENT:
            acs712_sensor = acs712_create(&acs712_config_continuous_measurement);
            break;
        default:
            acs712_sensor = acs712_create(&acs712_config_ordinary);
            break;
    }
}

void acs712SensorTearDown(void) {
    // Clean up code after each test
    if (acs712_sensor != NULL) {
        acs712_destroy(&acs712_sensor);
    }
}



TEST_CASE("acs712_current_sensor_test", "test_acs712_create") {
    acs712SensorSetUp(ACS712_TEST_CONFIG_ORDINARY);
    TEST_ASSERT_NOT_NULL(acs712_sensor);
    acs712SensorTearDown();
}

TEST_CASE("acs712_current_sensor_test", "test_acs712_init") {
    acs712SensorSetUp(ACS712_TEST_CONFIG_ORDINARY);
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    acs712SensorTearDown();
}

TEST_CASE("acs712_current_sensor_test", "test_acs712_deinit") {
    acs712SensorSetUp(ACS712_TEST_CONFIG_ORDINARY);
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = acs712_sensor_deinit(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    acs712SensorTearDown();
}

TEST_CASE("acs712_current_sensor_test", "test_acs712_destroy") {
    acs712SensorSetUp(ACS712_TEST_CONFIG_ORDINARY);
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = acs712_destroy(&acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(acs712_sensor); // Sensor should be NULL after destruction
}

TEST_CASE("acs712_current_sensor_test", "test_acs712_read_current") {
    acs712SensorSetUp(ACS712_TEST_CONFIG_ORDINARY);
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


TEST_CASE("acs712_current_sensor_test", "test_acs712_overcurrent_monitor") {
    acs712SensorSetUp(ACS712_TEST_CONFIG_OVERCURRENT_MONITOR);
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = acs712_monitor_current_window(acs712_sensor, OVERCURRENT_THRESHOLD_CURRENT,UNDERCURRENT_THRESHOLD_CURRENT, overcurrent_comparator_callback, NULL);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    acs712SensorTearDown();
}


TEST_CASE("acs712_current_sensor_test", "test_acs712_continuous_measurement") {
    acs712SensorSetUp(ACS712_TEST_CONFIG_CONTINUOUS_MEASUREMENT);
    error_type_t result = acs712_sensor_init(acs712_sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = acs712_monitor_read_current_with_cb(acs712_sensor, measurement_complete_callback_func, NULL);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    acs712SensorTearDown();
}