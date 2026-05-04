#include "unity.h"
#include <pump_monitor.h>
#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_random.h"

struct pump_config_t {
    int id; // Unique identifier for the pump
    const char* make; // Make/model of the pump
    float power_in_hp; // Power rating of the pump in horsepower
    float current_rating; // Current rating of the pump in amps
};

pump_config_t pump_config_for_testing = {
    .id = 1,
    .make = "Test Pump 1",
    .power_in_hp = 2.5, // GPIO pin to control the pump
    .current_rating = 6.0, // Maximum current rating of the pump in Amperes
    .min_working_current = 0.5 // Minimum working current of the pump in
};
struct current_sensor_t {
    int id; // Sensor ID
} ;
struct pump_t {
    int id; // Pump ID
};

struct pump_t mock_pump_instance = {
    .id = 1
};

struct current_sensor_t mock_current_sensor_instance = {
    .id = 1
};

const current_sensor_t *test_current_sensor  = &mock_current_sensor_instance;
pump_t *test_pump = &mock_pump_instance;

static const char *TAG = "TEST_PUMP_MONITOR";
static pump_monitor_t *pump_monitor = NULL;

static float mock_current_value = 6.23f;
static pump_state_machine_state_t pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;

/* Mock implementation (name/signature matches prototype used by pump_monitor.c) */
error_type_t current_sensor_get_current(struct current_sensor_t *sensor, float *current)
{
    if (!sensor || !current) {
        return SYSTEM_NULL_PARAMETER;
    }
    *current = mock_current_value;
    ESP_LOGI(TAG, "Mock Current sensor ID: %d, Current value: %.2f amp",
             sensor->id, (double)(*current));
    return SYSTEM_OK;
}

error_type_t dummy_current_analytics_callback(float* sampled_current_values, int number_of_samples, float rated_current, float min_working_current, pump_state_machine_state_t* state){
    ESP_LOGI("DUMMY_ANALYTICS_CALLBACK", "Current analytics callback triggered with %d samples, rated current: %.2f A, min working current: %.2f A", number_of_samples, rated_current, min_working_current);
    // generate random state for testing
    int random_value = esp_random() % 3; // Generate a random number between 0 and 2
    ESP_LOGI("DUMMY_ANALYTICS_CALLBACK", "Generated random state value: %d", random_value);
    *state = (pump_state_machine_state_t)random_value; // Cast the random number
    return SYSTEM_OK;
}

error_type_t pump_get_config(const pump_t *pump, pump_config_t *config){
    memcpy(config, &pump_config_for_testing, sizeof(pump_config_t));
    return SYSTEM_OK;
}


void dummy_pump_monitor_event_callback(void* context, event_type_t state,int monitor_id){
    ESP_LOGI("DUMMY_CALLBACK", "Pump monitor event callback triggered for monitor ID: %d with state: %d", monitor_id, state);
}

pump_monitor_config_t pump_monitor_config_for_testing = {
    .id = 1,
    .pump = test_pump, // Pointer to the pump being monitored (can be set to NULL for testing)
    .sensor = test_current_sensor, // Pointer to the current sensor (can be set to NULL for testing)
    .current_read_cb = (void*)current_sensor_get_current, // Callback function to read current value
    .number_of_samples_for_average = 20, // Number of samples to average for current reading
    .analytics_cb = dummy_current_analytics_callback // Callback function for current analytics
};


void pumpMonitorSetUp(void)
{
    test_pump = (pump_t *)malloc(sizeof(pump_t));
    test_pump->id = 456; // Assign a test ID
    test_current_sensor = (current_sensor_t *)malloc(sizeof(current_sensor_t));
    test_current_sensor->id = 123; // Assign a test ID
    pump_monitor = pump_monitor_create(pump_monitor_config_for_testing);
    TEST_ASSERT_NOT_NULL_MESSAGE(pump_monitor, "pump_monitor_create returned NULL");
}

void pumpMonitorTearDown(void)
{
    if (pump_monitor != NULL)
    {
        error_type_t err = pump_monitor_destroy(&pump_monitor);
        TEST_ASSERT_EQUAL_MESSAGE(SYSTEM_OK, err, "pump_monitor_destroy failed during tearDown");
        TEST_ASSERT_NULL_MESSAGE(pump_monitor, "pump_monitor should be NULL after destroy");
    }
    free(test_pump);
    free(test_current_sensor);
}



TEST_CASE("pump_monitor_test", "test_pump_monitor_create")
{
    pumpMonitorSetUp();
    TEST_ASSERT_NOT_NULL(pump_monitor);
    pump_monitor_state_t state;
    error_type_t result = pump_monitor_get_state(pump_monitor, &state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(PUMP_MONITOR_NOT_INITIALIZED, state);
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
    error_type_t result = pump_monitor_init(pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result = pump_monitor_deinit(pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    pump_monitor_state_t state;
    result = pump_monitor_get_state(pump_monitor, &state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(PUMP_MONITOR_NOT_INITIALIZED, state);

    pumpMonitorTearDown();
}

TEST_CASE("pump_monitor_test", "test_pump_monitor_destroy")
{
    pumpMonitorSetUp();

    error_type_t result = pump_monitor_destroy(&pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(pump_monitor);

}

TEST_CASE("pump_monitor_test", "test_pump_monitor_check_current")
{
    pumpMonitorSetUp();

    error_type_t result = pump_monitor_init(pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    int* subscriber_dummy_context_1 = NULL;
    pump_monitor_subscriber_t subscriber_one = {
        .id = 1,   
        .context = (void**)&subscriber_dummy_context_1,
        .callback = dummy_pump_monitor_event_callback
    };
    int event_id = -1;
    result = pump_monitor_subscribe_event(pump_monitor, &subscriber_one, &event_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, event_id);

    /* Normal case - should stay normal (no state change) */
    mock_current_value = 6.23f;
    pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;
    TEST_ASSERT_EQUAL(SYSTEM_OK, pump_monitor_check_current(pump_monitor));
    TEST_ASSERT_EQUAL(PUMP_STATE_MACHINE_NORMAL_STATE, pump_state_machine_state);

    /* Undercurrent */
    mock_current_value = 5.0f;
    pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;
    TEST_ASSERT_EQUAL(SYSTEM_OK, pump_monitor_check_current(pump_monitor));
    TEST_ASSERT_EQUAL(PUMP_STATE_MACHINE_UNDERCURRENT_STATE, pump_state_machine_state);

    /* Back to normal */
    mock_current_value = 6.23f;
    TEST_ASSERT_EQUAL(SYSTEM_OK, pump_monitor_check_current(pump_monitor));
    TEST_ASSERT_EQUAL(PUMP_STATE_MACHINE_NORMAL_STATE, pump_state_machine_state);

    /* Overcurrent */
    mock_current_value = 7.0f;
    pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;
    TEST_ASSERT_EQUAL(SYSTEM_OK, pump_monitor_check_current(pump_monitor));
    TEST_ASSERT_EQUAL(PUMP_STATE_MACHINE_OVERCURRENT_STATE, pump_state_machine_state);

    /* Null parameter check */
    TEST_ASSERT_EQUAL(SYSTEM_NULL_PARAMETER, pump_monitor_check_current(NULL));

    pumpMonitorTearDown();
}

TEST_CASE("pump_monitor_test", "test_pump_monitor_subscribe_event")
{
    pumpMonitorSetUp();

    error_type_t result = pump_monitor_init(pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    int* subscriber_dummy_context_1 = NULL;
    pump_monitor_subscriber_t subscriber_one = {
        .id = 1,   
        .context = (void**)&subscriber_dummy_context_1,
        .callback = dummy_pump_monitor_event_callback
    };
    int event_id = -1;
    result = pump_monitor_subscribe_event(pump_monitor, &subscriber_one, &event_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, event_id);

    /* Ensure subscribe didn't crash and the callback runs when state changes */
    mock_current_value = 5.0f; /* trigger undercurrent */
    pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;
    result = pump_monitor_check_current(pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(PUMP_STATE_MACHINE_UNDERCURRENT_STATE, pump_state_machine_state);

    pumpMonitorTearDown();
}

TEST_CASE("pump_monitor_test", "test_pump_monitor_unsubscribe_event")
{
    pumpMonitorSetUp();

    error_type_t result = pump_monitor_init(pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    int* subscriber_dummy_context_1 = NULL;
    pump_monitor_subscriber_t subscriber_one = {
        .id = 1,   
        .context = (void**)&subscriber_dummy_context_1,
        .callback = dummy_pump_monitor_event_callback
    };
    int event_id = -1;
    result = pump_monitor_subscribe_event(pump_monitor, &subscriber_one, &event_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, event_id);

    /* Unsubscribe should succeed */
    result = pump_monitor_unsubscribe_event(pump_monitor, event_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

  
    pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;
    mock_current_value = 5.0f;
    result = pump_monitor_check_current(pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(PUMP_STATE_MACHINE_NORMAL_STATE, pump_state_machine_state);

    pumpMonitorTearDown();
}
