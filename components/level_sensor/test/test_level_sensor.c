#include <level_sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"

level_sensor_t* level_Sensor = NULL;


void level_sensor_setup(void){
    rs485_config_t rs485 = {2, 17, 16, 4, 9600};
    level_sensor_config_t config = {4,rs485};
    level_Sensor = level_sensor_create(&config);
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