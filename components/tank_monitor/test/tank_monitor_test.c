#include "unity.h"
#include <tank_monitor.h>
#include <level_sensor.h>
#include <rs485.h>
#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include <rs485_context.h>
#include <protocol.h>

tank_monitor_t *monitor = NULL;
static const char *TAG = "TANK_MONITOR";
rs485_t *m_rs485_obj;
level_sensor_t *m_level_sensor;

tank_state_machine_state_t tank_state_machine_state = TANK_STATE_MACHINE_NORMAL_STATE;
void test_callback(void *context, int actuator_id, event_type_t event, int monitor_id)
{
    // Example callback function for tank monitor events
    ESP_LOGI(TAG, "Tank state changed to: %d for monitor ID: %d, actuator ID: %d\n", event, monitor_id, actuator_id);

    // Simulate changing the state based on the event
    if (event == EVENT_TANK_FULL_STATE)
    {
        tank_state_machine_state = TANK_STATE_MACHINE_FULL_STATE;
    }
    else if (event == EVENT_TANK_LOW_STATE)
    {
        tank_state_machine_state = TANK_STATE_MACHINE_LOW_STATE;
    }
    else
    {
        tank_state_machine_state = TANK_STATE_MACHINE_NORMAL_STATE;
    }
}

tank_monitor_event_hook_t test_hook = {
    .context = NULL,          // No context needed for this test
    .actuator_id = 1,         // Example actuator ID
    .callback = test_callback // Assign the test callback function
};

// comment the dummy_context_send_receive when using the real level sensor
 static error_type_t dummy_context_Send_receive(void *context, uint8_t *send_buff,int send_buff_size,
     uint8_t *receive_buff,int *receive_buff_size)
 {
     uint8_t valid_resp[] = { 0x01, 0x03, 0x02, 0x02, 0xF2, 0x38, 0xA1};
     memcpy(receive_buff, valid_resp, sizeof(valid_resp));
     *receive_buff_size = sizeof(valid_resp);
     return SYSTEM_OK;
 }

 
void tankMonitorSetUp(void)
{
    // Set up code before each test
    error_type_t err;
    tank_monitor_config_t config;
    config.tank = tank_create((tank_config_t){1, 1000.0, TANK_SHAPE_RECTANGLE, 100.0, 90, 10});
    // config.sensor = (level_sensor_t *)malloc(sizeof(level_sensor_t));
    rs485_config_t rs485_config = {2, 17, 16, 5, 9600};
    m_rs485_obj = rs485_create(&rs485_config);
    if(!m_rs485_obj){
        exit(1);
    }
    err = rs485_init(m_rs485_obj);
    if(err != SYSTEM_OK){
        exit(1);
    }
    protocol_callback_t protocol = protocol_gl_a01_read_level;
    send_receive_t send_receive =  dummy_context_Send_receive; //replace with the real context_send_receive
    // send_receive_t send_receive = rs485_context_send_receive;
    protocol_interpreter_t interpret = protocol_gl_a01_interpreter;
    level_sensor_config_t sensor_config = {.id = 4, .sensor_addr = 0x01, .protocol = protocol, .medium_context = m_rs485_obj, .send_recive = send_receive, .interpreter = interpret};
    m_level_sensor = level_sensor_create(sensor_config);
    if (m_level_sensor == NULL)
    {
       exit(1);
    }
    err = level_sensor_init(m_level_sensor);
    if(err != SYSTEM_OK){
        exit(1);
    }
    config.sensor = m_level_sensor;
    config.id = 1; // Example monitor ID
    monitor = tank_monitor_create(config);

}

void tankMonitorTearDown(void)
{
    // Clean up code after each test
    if (monitor != NULL)
    {
        tank_monitor_destroy(&monitor);
    }
    if(m_level_sensor){
        level_sensor_destroy(&m_level_sensor);
    }
    if(m_rs485_obj){
        rs485_destroy(&m_rs485_obj);
    }
}

TEST_CASE("tank_monitor_test", "test_tank_monitor_create")
{
    tankMonitorSetUp();
    TEST_ASSERT_NOT_NULL(monitor);
    tank_monitor_state_t state;
    error_type_t result = tank_monitor_get_state(monitor, &state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(TANK_MONITOR_NOT_INITIALIZED, state); // Initial state before initialization
    tankMonitorTearDown();
}

TEST_CASE("tank_monitor_test", "test_tank_monitor_init")
{
    tankMonitorSetUp();
    error_type_t result = tank_monitor_init(monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    tank_monitor_state_t state;
    result = tank_monitor_get_state(monitor, &state);
    TEST_ASSERT_EQUAL(TANK_MONITOR_INITIALIZED, state);
    tankMonitorTearDown();
}

TEST_CASE("tank_monitor_test", "test_tank_monitor_deinit")
{
    tankMonitorSetUp();
    tank_monitor_init(monitor);
    error_type_t result = tank_monitor_deinit(monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    tank_monitor_state_t state;
    result = tank_monitor_get_state(monitor, &state);
    TEST_ASSERT_EQUAL(TANK_MONITOR_NOT_INITIALIZED, state);
    tankMonitorTearDown();
}

TEST_CASE("tank_monitor_test", "test_tank_monitor_destroy")
{
    tankMonitorSetUp();
    error_type_t result = tank_monitor_destroy(&monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(monitor); // Monitor should be NULL after destruction
}

//This "test_tank_monitor_check_level" test below is without hardware 
TEST_CASE("tank_monitor_test", "test_tank_monitor_check_level")
{
    tankMonitorSetUp();
    tank_monitor_init(monitor);

    // Simulate checking the level sensor
    error_type_t result = tank_monitor_check_level(monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Check the state after checking level
    tank_monitor_config_t config;
    result = tank_monitor_get_config(monitor, &config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // state will be normal since  level sensor stub returns 90 and full level is 100
    TEST_ASSERT_EQUAL(TANK_STATE_MACHINE_NORMAL_STATE, tank_state_machine_state);
    

    tankMonitorTearDown();
}


TEST_CASE("tank_monitor_test", "test_tank_monitor_subscribe_state_event")
{
    tankMonitorSetUp();
    tank_monitor_init(monitor);

    int event_id;
    error_type_t result = tank_monitor_subscribe_state_event(monitor, &test_hook, &event_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NOT_EQUAL(-1, event_id); // Ensure event ID is valid

    // Simulate checking the level sensor to trigger the callback
    result = tank_monitor_check_level(monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Check if the callback was called and state changed
    TEST_ASSERT_EQUAL(TANK_STATE_MACHINE_FULL_STATE, tank_state_machine_state);

    tankMonitorTearDown();
}

TEST_CASE("tank_monitor_test", "test_tank_monitor_unsubscribe_state_event")
{
    tankMonitorSetUp();
    tank_monitor_init(monitor);

    int event_id;
    error_type_t result = tank_monitor_subscribe_state_event(monitor, &test_hook, &event_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Unsubscribe the event
    result = tank_monitor_unsubscribe_state_event(monitor, event_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Check if the callback is no longer called
    tank_state_machine_state = TANK_STATE_MACHINE_NORMAL_STATE; // Reset state
    result = tank_monitor_check_level(monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // State should not change since we unsubscribed
    TEST_ASSERT_EQUAL(TANK_STATE_MACHINE_NORMAL_STATE, tank_state_machine_state);

    tankMonitorTearDown();
}

TEST_CASE("tank_monitor_test", "test_tank_monitor_print_info_into_buffer")
{
    tankMonitorSetUp();
    error_type_t result;
    result = tank_monitor_init(monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    char *expected_buffer_content = "Tank ID: 1\n Subsciber Count: 0\n State: 1\n";

    char buffer[256];
    result = tank_monitor_print_info_into_buffer(monitor, buffer, 256);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    ESP_LOGI(TAG, "lenght of expected: %d\n", strlen(expected_buffer_content));
    ESP_LOGI(TAG, "lenght of actual: %d\n", strlen(buffer));
    ESP_LOGI(TAG, "\nExpected ouput:\n %s\n", expected_buffer_content);
    ESP_LOGI(TAG, "\nActual ouput:\n %s\n", buffer);
    int value = strcmp(buffer, expected_buffer_content);
    TEST_ASSERT_EQUAL(value, 0);
    tankMonitorTearDown();
}

//Uncomment this "test_tank_monitor_check_level" When testing with Hardware

// TEST_CASE("tank_monitor_test", "test_tank_monitor_check_level")
// {
//     tankMonitorSetUp();
//     tank_monitor_init(monitor);

//     // Simulate checking the level sensor
//     error_type_t result = tank_monitor_check_level(monitor);
//     TEST_ASSERT_EQUAL(SYSTEM_OK, result);

//     // Check the state after checking level
//     tank_monitor_config_t config;
//     result = tank_monitor_get_config(monitor, &config);
//     TEST_ASSERT_EQUAL(SYSTEM_OK, result);


//     tankMonitorTearDown();
// }