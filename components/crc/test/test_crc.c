#include <crc.h>
#include <stdio.h>
#include <stdint.h>
#include "unity.h"
#include "esp_log.h"

void crcSetup(void) {
    // Set up code before each test
}

void crctearDown(void) {
    // Clean up code after each test
}

static const char* CRC_TAG = "CRC";

TEST_CASE("crc_test", "test_crc"){
    crcSetup();
    uint16_t result;
    uint8_t buff[] ={0x01, 0x03, 0x01, 0x02, 0x00, 0x01,0x24, 0x36};
    result = MODBUS_CRC16_v3(buff,6);
    uint8_t result_lsb = result & 0xff;
    uint8_t result_msb = result >> 8;
    uint8_t expected_lsb = buff[6];  
    uint8_t expected_msb = buff[7]; 
    TEST_ASSERT_EQUAL(result_msb, expected_msb);
    ESP_LOGI(CRC_TAG, "result msb: 0x%X",result_msb );
    TEST_ASSERT_EQUAL(result_lsb, expected_lsb);
    ESP_LOGI(CRC_TAG, "result lsb: 0x%X", result_lsb);
    crctearDown();
}