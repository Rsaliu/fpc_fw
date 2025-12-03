#include <pump_control_unit.h>
#include <unity.h>
#include <common_headers.h>
#include <tank_monitor.h>
#include <level_sensor.h>
#include <rs485.h>
#include <stdbool.h>
#include <stdio.h>
#include <protocol.h>
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

pump_control_unit_t *pump_control_unit = NULL;

static const char* PUMP_CTRL_TAG = "PUMP_CONTROL_UNIT";

void pumpControlUnitSetUp(void) {
    // Set up code before each test
    pump_control_unit = pump_control_unit_create();
    TEST_ASSERT_NOT_NULL(pump_control_unit);
}

void pumpControlUnitTearDown(void) {
    // Clean up code after each test
    if (pump_control_unit != NULL) {
        pump_control_unit_destroy(&pump_control_unit);
    }
}

TEST_CASE("pump_control_unit_test", "test_pump_control_unit_create") {
    pumpControlUnitSetUp();
    TEST_ASSERT_NOT_NULL(pump_control_unit);
    bool is_initialized;
    error_type_t result = pump_control_unit_get_state(pump_control_unit, &is_initialized);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_FALSE(is_initialized); // Initial state before initialization
    pumpControlUnitTearDown();
}

TEST_CASE("pump_control_unit_test", "test_pump_control_unit_init") {
    pumpControlUnitSetUp();
    error_type_t result = pump_control_unit_init(pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    bool is_initialized;
    result = pump_control_unit_get_state(pump_control_unit, &is_initialized);
    TEST_ASSERT_TRUE(is_initialized);
    pumpControlUnitTearDown();
}

TEST_CASE("pump_control_unit_test", "test_pump_control_unit_deinit") {
    pumpControlUnitSetUp();
    error_type_t result = pump_control_unit_init(pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = pump_control_unit_deinit(pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    bool is_initialized;
    result = pump_control_unit_get_state(pump_control_unit, &is_initialized);
    TEST_ASSERT_FALSE(is_initialized);
    pumpControlUnitTearDown();
}

TEST_CASE("pump_control_unit_test", "test_pump_control_unit_destroy") {
    pumpControlUnitSetUp();
    error_type_t result = pump_control_unit_destroy(&pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(pump_control_unit); // Pump control unit should be NULL after destruction
}

TEST_CASE("pump_control_unit_test", "test_pump_control_unit_add_tank_monitor") {
    pumpControlUnitSetUp();
    tank_monitor_t *tank_monitor = tank_monitor_create((tank_monitor_config_t){.id = 1});
    TEST_ASSERT_NOT_NULL(tank_monitor);
    error_type_t result = pump_control_unit_init(pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = pump_control_unit_add_tank_monitor(pump_control_unit, tank_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Clean up
    tank_monitor_destroy(&tank_monitor);
    pumpControlUnitTearDown();
}

TEST_CASE("pump_control_unit_test", "test_pump_control_unit_remove_tank_monitor") {
    pumpControlUnitSetUp();
    tank_monitor_t *tank_monitor = tank_monitor_create((tank_monitor_config_t){.id = 1});
    TEST_ASSERT_NOT_NULL(tank_monitor);
    error_type_t result = pump_control_unit_init(pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = pump_control_unit_add_tank_monitor(pump_control_unit, tank_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    result = pump_control_unit_remove_tank_monitor(pump_control_unit, 1);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Clean up
    tank_monitor_destroy(&tank_monitor);
    pumpControlUnitTearDown();
}

TEST_CASE("pump_control_unit_test", "test_pump_control_unit_add_pump_monitor") {
    pumpControlUnitSetUp();
    error_type_t result = pump_control_unit_init(pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    pump_monitor_t *pump_monitor = (pump_monitor_t *)malloc(sizeof(pump_monitor_t));
    TEST_ASSERT_NOT_NULL(pump_monitor);
    pump_monitor->id = 1; // Example monitor ID
    pump_monitor->pump = NULL; // Set to NULL for this test
    
    result = pump_control_unit_add_pump_monitor(pump_control_unit, pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Clean up
    free(pump_monitor);
    pumpControlUnitTearDown();
}

TEST_CASE("pump_control_unit_test", "test_pump_control_unit_remove_pump_monitor") {
    pumpControlUnitSetUp();
    error_type_t result = pump_control_unit_init(pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    pump_monitor_t *pump_monitor = (pump_monitor_t *)malloc(sizeof(pump_monitor_t));
    TEST_ASSERT_NOT_NULL(pump_monitor);
    pump_monitor->id = 1; // Example monitor ID
    pump_monitor->pump = NULL; // Set to NULL for this test
    
    result = pump_control_unit_add_pump_monitor(pump_control_unit, pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    result = pump_control_unit_remove_pump_monitor(pump_control_unit, 1);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Clean up
    free(pump_monitor);
    pumpControlUnitTearDown();
}

TEST_CASE("pump_control_unit_test", "test_pump_control_unit_add_relay") {
    pumpControlUnitSetUp();
    error_type_t result = pump_control_unit_init(pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    relay_config_t relay_config = {.id = 1, .relay_pin_number = 1};
    relay_t *relay = relay_create(&relay_config);
    TEST_ASSERT_NOT_NULL(relay);
    
    result = pump_control_unit_add_relay(pump_control_unit, relay);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Clean up
    free(relay);
    pumpControlUnitTearDown();
}

TEST_CASE("pump_control_unit_test", "test_pump_control_unit_remove_relay") {
    pumpControlUnitSetUp();
    error_type_t result = pump_control_unit_init(pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    relay_config_t relay_config = {.id = 1, .relay_pin_number = 1};
    relay_t *relay = relay_create(&relay_config);
    TEST_ASSERT_NOT_NULL(relay);
    
    result = pump_control_unit_add_relay(pump_control_unit, relay);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    result = pump_control_unit_remove_relay(pump_control_unit, 1);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Clean up
    free(relay);
    pumpControlUnitTearDown();
}

TEST_CASE("pump_control_unit_test", "test_pump_control_unit_get_relay_by_id") {
    pumpControlUnitSetUp();
    error_type_t result = pump_control_unit_init(pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    relay_config_t relay_config = {.id = 1, .relay_pin_number = 1};
    relay_t *relay = relay_create(&relay_config);
    TEST_ASSERT_NOT_NULL(relay);

    
    result = pump_control_unit_add_relay(pump_control_unit, relay);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    relay_t *found_relay = NULL;
    result = pump_control_unit_get_relay_by_id(pump_control_unit, 1, &found_relay);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NOT_NULL(found_relay);
    
    // Clean up
    free(relay);
    pumpControlUnitTearDown();
}

TEST_CASE("pump_control_unit_test", "test_pump_control_unit_get_relay_by_invalid_id") {
    pumpControlUnitSetUp();
    error_type_t result = pump_control_unit_init(pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    relay_t *found_relay = NULL;
    result = pump_control_unit_get_relay_by_id(pump_control_unit, 999, &found_relay);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_PARAMETER, result); // Assuming 999 is an invalid ID
    TEST_ASSERT_NULL(found_relay);
    
    pumpControlUnitTearDown();
}
//comment the dummy_context_send_receive when using the real level sensor
static error_type_t dummy_context_Send_receive(void *context, uint8_t *send_buff,int send_buff_size,
    uint8_t *receive_buff,int *receive_buff_size)
{
    uint8_t valid_resp[] = { 0x01, 0x03, 0x02, 0x02, 0xF2, 0x38, 0xA1};
    memcpy(receive_buff, valid_resp, sizeof(valid_resp));
    *receive_buff_size = sizeof(valid_resp);
    return SYSTEM_OK;
}
// full end to end test
TEST_CASE("pump_control_unit_test", "test_pump_control_unit_full_end_to_end"){
    pumpControlUnitSetUp();
    error_type_t result = pump_control_unit_init(pump_control_unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    // Create a tank with a valid configuration
    tank_config_t tank_config = {
        .id = 1,
        .capacity_in_liters = 1000.0,
        .shape = TANK_SHAPE_RECTANGLE,
        .height_in_cm = 100.0,
        .full_level_in_mm = 90,
        .low_level_in_mm = 10
    };
    // create level sensor config
    rs485_config_t rs485_config = {2, 17, 16, 5, 9600};
    rs485_t* rs485_obj = rs485_create(&rs485_config);
    protocol_callback_t protocol = protocol_gl_a01_read_level;
    send_receive_t send_receive =  dummy_context_Send_receive; //replace with the real context_send_receive
    protocol_interpreter_t interpret = protocol_gl_a01_interpreter;
    level_sensor_config_t sensor_config = {.id = 4, .sensor_addr= 0x01, .protocol= protocol, .medium_context = rs485_obj, 
                                       .send_recive = send_receive,
                                       .interpreter = interpret };
    level_sensor_t* level_Sensor = level_sensor_create(sensor_config);
    TEST_ASSERT_NOT_NULL(level_Sensor);
    tank_t* my_tank = tank_create(tank_config);
    // Add a tank monitor
    TEST_ASSERT_NOT_NULL(my_tank);
    tank_monitor_config_t tank_monitor_config = {
        .id = 1,
        .tank = my_tank,
        .sensor = level_Sensor // Assuming level_sensor is initialized properly
    };
    tank_monitor_t *tank_monitor = tank_monitor_create(tank_monitor_config);
    TEST_ASSERT_NOT_NULL(tank_monitor);
    pump_config_t pump_config = {
        .id = 1,
        .power_in_hp = 5.0,
        .make = "TestPump"
    };
    // initialize a tank monitor
    error_type_t err = tank_monitor_init(tank_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    printf("Tank monitor pointer value: %p\n", tank_monitor);
    pump_t *pump = pump_create(pump_config);
    TEST_ASSERT_NOT_NULL(pump);

    current_sensor_t *current_sensor = (current_sensor_t *)malloc(sizeof(current_sensor_t));
    TEST_ASSERT_NOT_NULL(current_sensor);

    pump_monitor_t *pump_monitor = (pump_monitor_t *)malloc(sizeof(pump_monitor_t));
    TEST_ASSERT_NOT_NULL(pump_monitor);
    pump_monitor->id = 1; // Example monitor ID
    pump_monitor->pump = pump; // Set the pump for this monitor
    pump_monitor->current_sensor = current_sensor; // Assuming current_sensor is initialized properly

    result = pump_control_unit_add_tank_monitor(pump_control_unit, tank_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result = pump_control_unit_add_pump_monitor(pump_control_unit, pump_monitor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Add a relay
    relay_config_t relay_config = {.id = 1, .relay_pin_number = 1};
    relay_t *relay = relay_create(&relay_config);
    TEST_ASSERT_NOT_NULL(relay);

    result = pump_control_unit_add_relay(pump_control_unit, relay);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Get the relay by ID
    relay_t *found_relay = NULL;
    result = pump_control_unit_get_relay_by_id(pump_control_unit, 1, &found_relay);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NOT_NULL(found_relay);
    // get tank momitor state
    tank_monitor_state_t tank_monitor_state;
    result = tank_monitor_get_state(tank_monitor, &tank_monitor_state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(TANK_MONITOR_INITIALIZED, tank_monitor_state);
    ESP_LOGI(PUMP_CTRL_TAG,"Tank monitor state is initialized");
    // Add the relay to the tank monitor
    result = pump_control_add_relay_to_tank_monitor(pump_control_unit, 1, 1);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    ESP_LOGI(PUMP_CTRL_TAG,"Relay added to tank monitor successfully");
    // Remove the relay from the tank monitor
    result = pump_control_unit_remove_relay_from_tank_monitor(pump_control_unit, 1, 1);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    
    pumpControlUnitTearDown();
}


