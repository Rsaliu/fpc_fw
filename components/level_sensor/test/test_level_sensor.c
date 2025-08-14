#include <level_sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <protocol.h>
#include <rs485_context.h>
#include <rs485.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"
#include "esp_log.h"

level_sensor_t* level_Sensor = NULL;
rs485_t *rs485_obj;
static const char* TAG = "LEVEL_SENSOR_TEST";

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
    rs485_obj = rs485_create(&rs485_config);
    error_type_t err = rs485_init(rs485_obj);
    if (err != SYSTEM_OK) {
        ESP_LOGE(TAG,"RS485 init failed");
    }
    protocol_callback_t protocol = protocol_gl_a01_read_level;
    send_receive_t send_receive =  dummy_context_Send_receive;
    protocol_interpreter_t interpret = protocol_gl_a01_interpreter;
    level_sensor_config_t config = {.id = 4, .sensor_addr= 0x01, .protocol= protocol, .medium_context = rs485_obj, 
                                       .send_recive = send_receive,
                                       .interpreter = interpret,
                                    LEVEL_SENSOR_INTERFACE_RS485, GL_A01_PROTOCOL };
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
    ESP_LOGI(TAG,"create is sucessful");
    level_sensor_tearDown();
}

TEST_CASE("level_sensor_test", "test_level_sensor_init"){
    level_sensor_setup();
    error_type_t test_result;
    test_result = level_sensor_init(level_Sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_result);
    ESP_LOGI(TAG,"init is sucessful");
    level_sensor_tearDown();
}

TEST_CASE("level_sensor_test", "test_level_sensor_read"){
    level_sensor_setup();
    error_type_t test_result;
    uint16_t level_read_data;
    test_result = level_sensor_init(level_Sensor);
    test_result = level_sensor_read(level_Sensor, &level_read_data);
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_result);
    ESP_LOGI(TAG,"level data: %d\n", level_read_data);
    ESP_LOGI(TAG,"read buffer byte sucessfully");
    level_sensor_tearDown();

}

TEST_CASE("level_sensor_test", "test_level_sensor_deinit"){
    level_sensor_setup();
    error_type_t test_result = level_sensor_deinit(level_Sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK,test_result);
    ESP_LOGI(TAG,"deinit is sucessful \n");
    level_sensor_tearDown();

}

TEST_CASE("level_sensor_test", "test_level_sensor_destroy"){
    level_sensor_setup();
    error_type_t test_result = level_sensor_destroy(&level_Sensor);
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_result);
    ESP_LOGI(TAG,"destroy is sucessful");
    TEST_ASSERT_NULL(level_Sensor);
}
 
// on comment when  you are using it with the level sensor Hardware  

// void real_level_sensor_setup(void){
//     protocol_callback_t protocol = protocol_gl_a01_read_level;
//     send_receive_t send_receive =  rs485_context_send_receive;
//     protocol_interpreter_t interpret = protocol_gl_a01_interpreter;
//     level_sensor_config_t config = {.id = 4, .sensor_addr= 0x01, .protocol= protocol, 
//                                        .medium_context = rs485_obj, 
//                                        .send_recive = send_receive,
//                                        .interpreter = interpret };
//     level_Sensor = level_sensor_create(config);
// }


// TEST_CASE("level_sensor_test", "test_real_level_sensor_setup"){
//     real_level_sensor_setup();
//     error_type_t test_result;
//     uint16_t level_read_data;
//     test_result = level_sensor_init(level_Sensor);
//     for (;;) // forever for loop
//     {
//         test_result = level_sensor_read(level_Sensor, &level_read_data);
//         TEST_ASSERT_EQUAL(SYSTEM_OK, test_result);
//         ESP_LOGI(TAG,"level data: %dmm\n", level_read_data);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
    
//     level_sensor_tearDown();

// }

