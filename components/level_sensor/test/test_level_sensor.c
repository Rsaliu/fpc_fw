#include <level_sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <protocol.h>
#include <rs485_context.h>
#include <rs485.h>
#include "unity.h"

level_sensor_t* level_Sensor = NULL;

static error_type_t dummy_context_Send_receive(void *context, uint8_t *send_buff,int send_buff_size,
    uint8_t *receive_buff,int *receive_buff_size)
{
    uint8_t valid_resp[] = { 0x01, 0x03, 0x02, 0x02, 0xF2, 0x38, 0xA1};
    memcpy(receive_buff, valid_resp, sizeof(valid_resp));
    *receive_buff_size = sizeof(valid_resp);
    return SYSTEM_OK;
}

void level_sensor_setup(void){
    rs485_config_t rs485_config = {2,17,16,4,9600};
    rs485_t *rs485 = rs485_create(&rs485_config);
    protocol_callback_t protocol = protocol_gl_a01_read_level;
    send_receive_t send_receive =  dummy_context_Send_receive;
    protocol_interpreter_t interpret = protocol_gl_a01_interpreter;
    level_sensor_config_t config = {.id = 4, .sensor_addr= 0x01, .protocol= protocol, .medium_context = rs485, 
                                       .send_recive = send_receive,
                                       .interpreter = interpret };
    level_Sensor = level_sensor_create(config);
}

void level_sensor_tearDown(void){
    if (level_Sensor != NULL)
    {
        level_sensor_destroy(&level_Sensor);
    }
}

TEST_CASE("level_sensor_test", "test_level_sensor_create"){
    level_sensor_setup();
    TEST_ASSERT_NOT_NULL(level_Sensor);
    printf("create is sucessful");
    level_sensor_tearDown();
}

TEST_CASE("level_sensor_test", "test_level_sensor_init"){
    level_sensor_setup();
    error_type_t test_result;
    test_result = level_sensor_init(level_Sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_result);
    printf("init is sucessful\n");
    level_sensor_tearDown();
}

TEST_CASE("level_sensor_test", "test_level_sensor_read"){
    level_sensor_setup();
    error_type_t test_result;
    test_result = level_sensor_init(level_Sensor);
    test_result = level_sensor_read(level_Sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_result);
    printf("read buffer byte sucessfully");
    level_sensor_tearDown();

}

TEST_CASE("level_sensor_test", "test_level_sensor_deinit"){
    level_sensor_setup();
    error_type_t test_result = level_sensor_deinit(level_Sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK,test_result);
    printf("deinit is sucessful \n");
    level_sensor_tearDown();

}

TEST_CASE("level_sensor_test", "test_level_sensor_destroy"){
    level_sensor_setup();
    error_type_t test_result = level_sensor_destroy(&level_Sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_result);
    printf("destroy is sucessful");
    TEST_ASSERT_NULL(level_Sensor);
}