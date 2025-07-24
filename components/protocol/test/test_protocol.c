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

TEST_CASE("protocol_test", "test_protocol_gl_a01_write_address"){
    protocolSetup();
    uint8_t current_addr= 0x01;
    uint8_t new_addr = 0x05;
    uint8_t buffer [8];
    uint8_t buffer_size = sizeof(buffer);
    error_type_t err = protocol_gl_a01_write_address(current_addr,new_addr,buffer,buffer_size);
    TEST_ASSERT_EQUAL(SYSTEM_OK,err);
    printf("write address was successful\n");
    protocoltearDown();
}

TEST_CASE("protocol_test", "test_protocol_gl_a01_read_level"){
    protocolSetup();
    uint8_t slave_addr = 0x01;
    uint8_t buffer[8];
    int buffer_size = sizeof(buffer);
    uint8_t payload_size = 0;
    error_type_t err = protocol_gl_a01_read_level(slave_addr,buffer, buffer_size, &payload_size);
    TEST_ASSERT_EQUAL(SYSTEM_OK,err);
    TEST_ASSERT_EQUAL(8, payload_size);
    printf(" read protocol was successful\n");
    protocoltearDown();
}

TEST_CASE("protocol_test", "test_protocol_gl_a01_read_temp"){
    protocolSetup();
    uint8_t slave_addr = 0x01;
    uint8_t buffer[8];
    uint8_t buffer_size = sizeof(buffer);
    error_type_t err = protocol_gl_a01_read_temp(slave_addr,buffer,buffer_size);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    printf("read temperature protocol is sucessful\n");
    protocoltearDown();

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