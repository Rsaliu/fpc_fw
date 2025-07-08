#include <tank.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"

tank_t *tank = NULL;
void tankSetUp(void) {
    // Set up code before each test
    tank_config_t config = {1, 1000.0, TANK_SHAPE_RECTANGLE, 100.0, 90, 10}; // Example configuration
    tank = tank_create(config);
}
void tankTearDown(void) {
    // Clean up code after each test
    if (tank != NULL) {
        tank_destroy(&tank);
    }
}

TEST_CASE("tank_test", "test_tank_create") {
    tankSetUp();
    error_type_t result;
    TEST_ASSERT_NOT_NULL(tank);
    result = tank_get_config(tank, NULL);
    TEST_ASSERT_EQUAL(SYSTEM_NULL_PARAMETER, result);
    tankTearDown();
}

TEST_CASE("tank_test", "test_tank_init") {
    tankSetUp();
    error_type_t result = tank_init(tank);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    tank_state_t state;
    tank_get_state(tank, &state);
    TEST_ASSERT_EQUAL(TANK_INITIALIZED, state);
    tankTearDown();
}

TEST_CASE("tank_test", "test_tank_get_state") {
    tankSetUp();
    tank_state_t state;
    error_type_t result = tank_get_state(tank, &state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(TANK_NOT_INITIALIZED, state); // Initial state before initialization
    tankTearDown();
}

TEST_CASE("tank_test", "test_tank_deinit") {
    tankSetUp();
    tank_init(tank);
    error_type_t result = tank_deinit(tank);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    tank_state_t state;
    tank_get_state(tank, &state);
    TEST_ASSERT_EQUAL(TANK_NOT_INITIALIZED, state);
    tankTearDown();
}

TEST_CASE("tank_test", "test_tank_destroy") {
    tankSetUp();
    error_type_t result = tank_destroy(&tank);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(tank);
}

TEST_CASE("tank_test", "test_tank_get_config") {
    tankSetUp();
    tank_config_t config;
    error_type_t result = tank_get_config(tank, &config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(1, config.id);
    TEST_ASSERT_EQUAL_FLOAT(1000.0, config.capacity_in_liters);
    TEST_ASSERT_EQUAL(TANK_SHAPE_RECTANGLE, config.shape);
    TEST_ASSERT_EQUAL_FLOAT(100.0, config.height_in_cm);
    TEST_ASSERT_EQUAL(90, config.full_level_in_mm);
    TEST_ASSERT_EQUAL(10, config.low_level_in_mm);
    tankTearDown();
}

