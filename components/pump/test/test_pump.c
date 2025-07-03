#include <pump.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"
 
pump_t *pump = NULL;
void setUp(void) {
    // Set up code before each test
    pump_config_t config = {1, "Test Pump", 5.0, {1}}; // Example configuration
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

TEST_CASE("pump_test", "test_pump_start") {
    setUp();
    pump_init(pump);
    error_type_t result = pump_start(pump);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    pump_state_t state;
    result = pump_get_state(pump, &state);
    TEST_ASSERT_EQUAL(PUMP_ON, state);
    tearDown();
}

TEST_CASE("pump_test", "test_pump_stop") {
    setUp();
    pump_init(pump);
    pump_start(pump);
    error_type_t result = pump_stop(pump);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    pump_state_t state;
    result = pump_get_state(pump, &state);
    TEST_ASSERT_EQUAL(PUMP_OFF, state);
    tearDown();
}

TEST_CASE("pump_test", "test_pump_deinit") {
    setUp();
    pump_init(pump);
    pump_start(pump);
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