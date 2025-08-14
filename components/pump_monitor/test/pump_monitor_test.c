#include "unity.h"
#include <pump_monitor.h>
#include <string.h>
#include <stdio.h>
#include "esp_log.h"


static const char *TAG = "TEST_PUMP_MONITOR";
pump_monitor_t *pump_monitor = NULL;

static float mock_current_value = 6.23f;



error_type_t current_sensor_get_current(current_sensor_t *sensor, float *current)
{
    if (!sensor || !current) {
        return SYSTEM_NULL_PARAMETER;
    }
    *current = mock_current_value;
    ESP_LOGI(TAG, "Mock Current sensor ID: %d, Current value: %.2f amp",
             sensor->id, (double)(*current));
    return SYSTEM_OK;
}

static pump_state_machine_state_t pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;


void pump_test_callback(void *context, int actuator_id, event_type_t event, int pump_monitor_id)
{
    
    ESP_LOGI(TAG, "Pump state changed to: %d for pump_monitor ID: %d, actuator ID: %d", event, pump_monitor_id, actuator_id);
    
    if (event == EVENT_PUMP_NORMAL)
    {
        pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;
    }
    else if (event == EVENT_PUMP_UNDERCURRENT)
    {
        pump_state_machine_state = PUMP_STATE_MACHINE_UNDERCURRENT_STATE;
    }
    else
    {
        pump_state_machine_state = PUMP_STATE_MACHINE_OVERCURRENT_STATE;
    }
    ESP_LOGI(TAG, "Updated pump_state_machine_state: %d", pump_state_machine_state);
}

pump_monitor_event_hook_t pump_test_hook = {
    .context = NULL,               // No context needed for this test
    .actuator_id = 1,              // Example actuator ID
    .callback = pump_test_callback // Assign the test callback function
};

void pumpMonitorSetUp(void)
{
    // Set up code before each test
    pump_monitor_config_t config;

    config.pump = pump_create((pump_config_t){
        .id = 1,
        .make = "Test Pump",
        .power_in_hp = 2.0f,
        .current_rating = 6.23f});

    
    
    config.sensor = malloc(sizeof(current_sensor_t));
    config.id = 1;
    config.sensor->id = 1;

    pump_monitor = pump_monitor_create(config);
}

void pumpMonitorTearDown(void)
{
  
    if (pump_monitor != NULL)
    {
        pump_monitor_destroy(&pump_monitor);
    }
}

TEST_CASE("pump_monitor_test", "test_pump_monitor_create")
{
    pumpMonitorSetUp();
    TEST_ASSERT_NOT_NULL(pump_monitor);
    pump_monitor_state_t state;
    error_type_t result = pump_monitor_get_state(pump_monitor, &state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(PUMP_MONITOR_NOT_INITIALIZED, state); // Initial state before initialization
    pumpMonitorTearDown();
}

TEST_CASE("pump_monitor_test", "test_pump_monitor_init")
{
    pumpMonitorSetUp();
    error_type_t result = pump_monitor_init(pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    pump_monitor_state_t state;
    result = pump_monitor_get_state(pump_monitor, &state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(PUMP_MONITOR_INITIALIZED, state);
    pumpMonitorTearDown();
}

TEST_CASE("pump_monitor_test", "test_pump_monitor_deinit")
{
    pumpMonitorSetUp();
    pump_monitor_init(pump_monitor);
    error_type_t result = pump_monitor_deinit(pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    pump_monitor_state_t state;
    result = pump_monitor_get_state(pump_monitor, &state);
    TEST_ASSERT_EQUAL(PUMP_MONITOR_NOT_INITIALIZED, state);
    pumpMonitorTearDown();
}

TEST_CASE("pump_monitor_test", "test_pump_monitor_destroy")
{
    pumpMonitorSetUp();
    error_type_t result = pump_monitor_destroy(&pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(pump_monitor); // pump_monitor should be NULL after destruction
}


TEST_CASE("pump_monitor_test", "test_pump_monitor_check_current")
{
    pumpMonitorSetUp();
    pump_monitor_init(pump_monitor);

    int event_id;
    pump_monitor_subscribe_event(pump_monitor, &pump_test_hook, &event_id);

    // Normal case
    mock_current_value = 6.23f;
    pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;
    TEST_ASSERT_EQUAL(SYSTEM_OK, pump_monitor_check_current(pump_monitor));
    TEST_ASSERT_EQUAL(PUMP_STATE_MACHINE_NORMAL_STATE, pump_state_machine_state);

    // Undercurrent
    mock_current_value = 5.0f;
    pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;
    TEST_ASSERT_EQUAL(SYSTEM_OK, pump_monitor_check_current(pump_monitor));
    TEST_ASSERT_EQUAL(PUMP_STATE_MACHINE_UNDERCURRENT_STATE, pump_state_machine_state);

    // Back to normal
    mock_current_value = 6.23f;
    TEST_ASSERT_EQUAL(SYSTEM_OK, pump_monitor_check_current(pump_monitor));
    TEST_ASSERT_EQUAL(PUMP_STATE_MACHINE_NORMAL_STATE, pump_state_machine_state);

    // Overcurrent
    mock_current_value = 7.0f;
    pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;
    TEST_ASSERT_EQUAL(SYSTEM_OK, pump_monitor_check_current(pump_monitor));
    TEST_ASSERT_EQUAL(PUMP_STATE_MACHINE_OVERCURRENT_STATE, pump_state_machine_state);

    // Null parameter check
    TEST_ASSERT_EQUAL(SYSTEM_NULL_PARAMETER, pump_monitor_check_current(NULL));

    pumpMonitorTearDown();
}

TEST_CASE("pump_monitor_test", "test_pump_monitor_subscribe_event")
{
    pumpMonitorSetUp();
    pump_monitor_init(pump_monitor);

    int event_id;
    error_type_t result = pump_monitor_subscribe_event(pump_monitor, &pump_test_hook, &event_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NOT_EQUAL(-1, event_id); // Ensure event ID is valid

    // Simulate checking the level sensor to trigger the callback
    result = pump_monitor_check_current(pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    pumpMonitorTearDown();
}

TEST_CASE("pump_monitor_test", "test_pump_monitor_unsubscribe_event")
{
    pumpMonitorSetUp();
    pump_monitor_init(pump_monitor);

    int event_id;
    error_type_t result = pump_monitor_subscribe_event(pump_monitor, &pump_test_hook, &event_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Unsubscribe the event
    result = pump_monitor_unsubscribe_event(pump_monitor, event_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Check if the callback is no longer called
    pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE; // Reset state
    result = pump_monitor_check_current(pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // State should not change since we unsubscribed
    TEST_ASSERT_EQUAL(PUMP_STATE_MACHINE_NORMAL_STATE, pump_state_machine_state);

    pumpMonitorTearDown();
}
