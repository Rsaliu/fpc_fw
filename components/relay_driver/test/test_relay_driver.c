#include <relay_driver.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"

relay_t *relay = NULL;

void relaySetUp(void) {
    // relaySetup before each test
    relay_config_t config = { .relay_pin_number = 5 }; // Replace with valid GPIO
    relay = relay_create(&config);
}

void relayTearDown(void) {
    // Cleanup after each test
    if (relay != NULL) {
        relay_destroy(&relay);
    }
}

TEST_CASE("relay_test", "test_relay_create") {
    relaySetUp();
    TEST_ASSERT_NOT_NULL(relay);
    printf("relay create sucessful");
    relayTearDown();
}

TEST_CASE("relay_test", "test_relay_init") {
    relaySetUp();
    error_type_t result = relay_init(relay);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    relay_state_t state;
    result = relay_check_state(relay,&state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(RELAY_OFF, state);
    printf("relay init sucessful");
    relayTearDown();
}

TEST_CASE("relay_test", "test_relay_deinit") {
    relaySetUp();
    error_type_t result = relay_init(relay);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    relay_state_t state = RELAY_ON; 
    result = relay_switch(relay, state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = relay_deinit(relay);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = relay_check_state(relay,&state);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_STATE, result);
    printf("relay deinit sucessful");
    relayTearDown();
}

TEST_CASE("relay_test", "test_relay_switch") {
    relaySetUp();
    error_type_t result;
    result = relay_init(relay);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    relay_state_t state = RELAY_ON; 
    result = relay_switch(relay, state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = relay_check_state(relay,&state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(RELAY_ON, state);
    printf("switch on sucessful");
    state = RELAY_OFF; 
    result = relay_switch(relay, state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = relay_check_state(relay,&state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(RELAY_OFF, state);
    printf("switch off sucessful");
    relayTearDown();
}

TEST_CASE("relay_test", "test_relay_destroy") {
    relaySetUp();
    relayTearDown();
    TEST_ASSERT_NULL(relay);
}

TEST_CASE("relay_test", "test_relay_get_config") {
    relay_config_t config = { .id = 1,.relay_pin_number = 5 }; // Replace with valid GPIO
    relay = relay_create(&config);
    relay_config_t read_config;
    error_type_t err = relay_get_config(relay,&read_config);
    TEST_ASSERT_EQUAL(err,SYSTEM_OK);
    TEST_ASSERT_EQUAL(config.id,read_config.id);
    TEST_ASSERT_EQUAL(config.relay_pin_number,config.relay_pin_number);
    relayTearDown();
}

// TEST_CASE("relay_test", "test_relay_check_state") {
//     relaySetUp();
//     relay_init(relay);
//     relay_switch(relay, RELAY_ON);
//     relay_state_t state = RELAY_OFF;

//     error_type_t result = relay_check_state(relay, &state);
//     TEST_ASSERT_EQUAL(SYSTEM_OK, result);
//     TEST_ASSERT_EQUAL(RELAY_ON, state);
//     relayTearDown();
// }

// TEST_CASE("relay_test", "test_relay_destroy") {
//     relaySetUp();
//     error_type_t result = relay_destroy(&relay);
//     TEST_ASSERT_EQUAL(SYSTEM_OK, result);
//     TEST_ASSERT_NULL(relay);

//     // Try to check state after destroy
//     relay_state_t state;
//     result = relay_check_state(relay, &state);
//     TEST_ASSERT_EQUAL(SYSTEM_NULL_PARAMETER, result);
// }
