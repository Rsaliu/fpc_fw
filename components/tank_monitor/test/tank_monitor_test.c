#include "unity.h"
#include <tank_monitor.h>
#include <string.h>
#include <stdio.h>

tank_monitor_t *monitor = NULL;

tank_state_machine_state_t tank_state_machine_state = TANK_STATE_MACHINE_NORMAL_STATE;
void test_callback(tank_state_machine_state_t state) {
    // Example callback function for tank monitor events
    printf("Tank state changed to: %d\n", state);
    tank_state_machine_state = state;
}
void tankMonitorSetUp(void) {
    // Set up code before each test
    tank_monitor_config_t config;
    config.tank = tank_create((tank_config_t){1, 1000.0, TANK_SHAPE_RECTANGLE, 100.0, 90, 10});
    config.sensor = (level_sensor_t *)malloc(sizeof(level_sensor_t));
    config.sensor->id = 1; // Example sensor ID
    config.event_callback = test_callback; // No callback for simplicity
    monitor = tank_monitor_create(config);
}

void tankMonitorTearDown(void) {
    // Clean up code after each test
    if (monitor != NULL) {
        tank_monitor_destroy(&monitor);
    }
}

TEST_CASE("tank_monitor_test", "test_tank_monitor_create") {
    tankMonitorSetUp();
    TEST_ASSERT_NOT_NULL(monitor);
    tank_monitor_state_t state;
    error_type_t result = tank_monitor_get_state(monitor, &state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(TANK_MONITOR_NOT_INITIALIZED, state); // Initial state before initialization
    tankMonitorTearDown();
}

TEST_CASE("tank_monitor_test", "test_tank_monitor_init") {
    tankMonitorSetUp();
    error_type_t result = tank_monitor_init(monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    tank_monitor_state_t state;
    result = tank_monitor_get_state(monitor, &state);
    TEST_ASSERT_EQUAL(TANK_MONITOR_INITIALIZED, state);
    tankMonitorTearDown();
}

TEST_CASE("tank_monitor_test", "test_tank_monitor_deinit") {
    tankMonitorSetUp();
    tank_monitor_init(monitor);
    error_type_t result = tank_monitor_deinit(monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    tank_monitor_state_t state;
    result = tank_monitor_get_state(monitor, &state);
    TEST_ASSERT_EQUAL(TANK_MONITOR_NOT_INITIALIZED, state);
    tankMonitorTearDown();
}

TEST_CASE("tank_monitor_test", "test_tank_monitor_destroy") {
    tankMonitorSetUp();
    error_type_t result = tank_monitor_destroy(&monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(monitor); // Monitor should be NULL after destruction
}

TEST_CASE("tank_monitor_test", "test_tank_monitor_check_level") {
    tankMonitorSetUp();
    tank_monitor_init(monitor);
    
    // Simulate checking the level sensor
    error_type_t result = tank_monitor_check_level(monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Check the state after checking level
    tank_monitor_config_t config;
    result = tank_monitor_get_config(monitor, &config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // state will be full since  level sensor stub returns 100 and full level is 90
    TEST_ASSERT_EQUAL(TANK_STATE_MACHINE_FULL_STATE, tank_state_machine_state);
    
    tankMonitorTearDown();
}