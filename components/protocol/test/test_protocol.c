#include <protocol.h>
#include <stdio.h>
#include <stdint.h>
#include <crc.h>
#include "unity.h"

void protocolSetup(void) {
    // Set up code before each test
}

void protocoltearDown(void) {
    // Clean up code after each test
}

TEST_CASE("protocol_test","test_protocol_gl_a01_interpreter"){
    protocolSetup();
    error_type_t err;
    
    uint8_t buffer[] = { 0x01, 0x03, 0x02, 0x02, 0xF2, 0x38, 0xA1};
    int buff_size = sizeof(buffer);

    uint16_t sensor_data = 754;
    uint16_t expected_data;
    err = protocol_gl_a01_interpreter(buffer, buff_size, &expected_data);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    TEST_ASSERT_EQUAL(expected_data, sensor_data);

    protocoltearDown();
}