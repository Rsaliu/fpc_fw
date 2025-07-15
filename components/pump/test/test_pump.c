#include <pump.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"
 
pump_t *pump = NULL;
void setUp(void) {
    // Set up code before each test
    pump_config_t config = {1, "Test Pump", 5.0}; // Example configuration
    pump = pump_create(config);  
}

void tearDown(void) {
    // Clean up code after each test
    if (pump != NULL) {
        pump_destroy(&pump);
    }
}




TEST_CASE("pump_test", "test_pump_create") {
    setUp();
    pump_state_t state;
    error_type_t result;
    TEST_ASSERT_NOT_NULL(pump);
    result = pump_get_state(pump, &state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    tearDown();
}

TEST_CASE("pump_test", "test_pump_init") {
    setUp();
    error_type_t result = pump_init(pump);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    pump_state_t state;
    result = pump_get_state(pump, &state);
    TEST_ASSERT_EQUAL(PUMP_INITIALIZED, state);
    tearDown();
}



TEST_CASE("pump_test", "test_pump_deinit") {
    setUp();
    pump_init(pump);
    error_type_t result = pump_deinit(pump);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    pump_state_t state;
    result = pump_get_state(pump, &state);
    TEST_ASSERT_EQUAL(PUMP_NOT_INITIALIZED, state);
    tearDown();
}

TEST_CASE("pump_test", "test_pump_destroy") {
    setUp();
    pump_state_t state;
    error_type_t result = pump_destroy(&pump);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(pump);
    result = pump_get_state(pump, &state);
    TEST_ASSERT_EQUAL(SYSTEM_NULL_PARAMETER, result); // Expecting null parameter error
}

TEST_CASE("pump_test", "test_pump_get_config") {
    setUp();
    pump_config_t config;
    error_type_t result = pump_get_config(pump, &config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);   
    TEST_ASSERT_EQUAL(1, config.id);
    TEST_ASSERT_EQUAL_STRING("Test Pump", config.make);
    TEST_ASSERT_EQUAL_FLOAT(5.0, config.power_in_hp);
}

// test invalid configurations
TEST_CASE("pump_test", "test_pump_init_invalid_config") {
    //setUp();
    pump_config_t invalid_config = {-1, NULL, -5.0}; // Invalid configuration
    pump_t *invalid_pump = pump_create(invalid_config);
    TEST_ASSERT_NOT_NULL(invalid_pump);
    
    error_type_t result = pump_init(invalid_pump);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result); // Expecting invalid parameter error
    
    pump_destroy(&invalid_pump);
}