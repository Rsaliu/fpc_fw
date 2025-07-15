#include <rs485.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"

 rs485_t* rs485 = NULL;


void rs485_setup(void){
    rs485_config_t config ={2,17,16,4,9600};
    rs485 = rs485_create(&config);
}

void rs485_tearDown(void){
    if (rs485 != NULL)
    {
        rs485_destroy(&rs485);
    }
}

TEST_CASE(" rs485_test", "test_rs485_create"){
    rs485_setup();
    TEST_ASSERT_NOT_NULL(rs485);
    printf("create is sucessful");
    rs485_tearDown();
}

TEST_CASE(" rs485_test", "test_rs485_init"){
    rs485_setup();
    error_type_t test_result;
    test_result = rs485_init(rs485);
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_result);
    printf("init is sucessful\n");
    rs485_tearDown();
}

TEST_CASE("leve_sensor_test", "test_rs485_write"){
    rs485_setup();
    error_type_t test_result;
    test_result = rs485_init(rs485);
    TEST_ASSERT_EQUAL(test_result,SYSTEM_OK);
    char data [] = {1,2,3,4};
    test_result = rs485_write(rs485, data, 4);
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_result);
    printf("write sucessfully\n");
    rs485_tearDown();
}

TEST_CASE(" rs485_test", "test_rs485_read"){
    rs485_setup();
    rs485_config_t config ={1,32,33,5,9600};
    rs485_t *rs4852 = rs485_create(&config);
    TEST_ASSERT_NOT_NULL(rs485);
    TEST_ASSERT_NOT_NULL(rs4852);
    error_type_t test_result;
    test_result = rs485_init(rs485);
    TEST_ASSERT_EQUAL(test_result,SYSTEM_OK);
    test_result = rs485_init(rs4852);
    TEST_ASSERT_EQUAL(test_result,SYSTEM_OK);
    char data [] = {1,2,3,4};
    char read_buffer[4];
    int read_size;

    test_result = rs485_write(rs485, data, 4);
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_result);

    test_result = rs485_read(rs4852, read_buffer, 4,&read_size);
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_result);
    printf("read size: %d\n", read_size);
    TEST_ASSERT_EQUAL(read_size,4);
    for(int x=0;x<4;x++){

        TEST_ASSERT_EQUAL(data[x],read_buffer[x]);
        printf("data and read buffer is equal\n");
    }
    
    printf("read sucessfully\n");
    rs485_tearDown();
    rs485_destroy(&rs4852);
}


TEST_CASE(" rs485_test", "test_rs485_deinit"){
    rs485_setup();
    error_type_t test_result = rs485_deinit(rs485);
    TEST_ASSERT_EQUAL(SYSTEM_OK,test_result);
    printf("deinit is sucessful \n");
    rs485_tearDown();

}

TEST_CASE(" rs485_test", "test_rs485_destroy"){
    rs485_setup();
    error_type_t test_result = rs485_destroy(&rs485);
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_result);
    printf("destroy is sucessful");
    TEST_ASSERT_NULL(rs485);
}