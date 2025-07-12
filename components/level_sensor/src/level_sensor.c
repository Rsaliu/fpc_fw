#include <level_sensor.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#define BUF_SIZE    (127)



struct level_sensor_t
{
    int uart_num;
    int rs485_ro_pin; 
    int rs485_di_pin;
    int rs485_dir_pin;
    int baud_rate;
    bool activate;
};

level_sensor_t* level_sensor_create(const level_sensor_config_t* config){
    if (config == NULL)
    {
        return NULL;
    }

    level_sensor_t* level_sensor_obj = (level_sensor_t*)malloc(sizeof(level_sensor_t));
    level_sensor_obj->uart_num = config->uart_num;
    level_sensor_obj->rs485_ro_pin = config->rs485_ro_pin;
    level_sensor_obj->rs485_di_pin = config->rs485_di_pin;
    level_sensor_obj->rs485_dir_pin = config->rs485_dir_pin;
    level_sensor_obj->baud_rate = config->baud_rate;
    level_sensor_obj->activate = false;

    return level_sensor_obj;   
}

error_type_t level_sensor_init(level_sensor_t* level_sensor_obj){
    if(level_sensor_obj == NULL)return SYSTEM_NULL_PARAMETER;

    uart_config_t uart_config = {
    .baud_rate = level_sensor_obj->baud_rate,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_APB,
   };

    
    // configure uart pin

    uart_param_config(level_sensor_obj->uart_num, &uart_config);
    uart_set_pin(level_sensor_obj->uart_num, level_sensor_obj->rs485_ro_pin ,level_sensor_obj->rs485_di_pin, UART_PIN_NO_CHANGE , UART_PIN_NO_CHANGE );
    uart_driver_install(level_sensor_obj->uart_num, BUF_SIZE*2,0,0,NULL,0);

    //configure gpio pin
    gpio_set_direction(level_sensor_obj->rs485_dir_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(level_sensor_obj->rs485_dir_pin, 0); // 0 -> LOW, 1 -> HIGH
                                      // set the rs485 to receiver mode  
    level_sensor_obj->activate = true;

    return SYSTEM_OK;
}

error_type_t rs485_write(level_sensor_t* level_sensor_obj, const char* data, size_t buffer_size){
    if (level_sensor_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    if(!level_sensor_obj->activate){
        return SYSTEM_INVALID_STATE;
    }
    gpio_set_level(level_sensor_obj->rs485_dir_pin, 1);
    uart_write_bytes(level_sensor_obj->uart_num, data, buffer_size);
    uart_wait_tx_done(level_sensor_obj->uart_num, portMAX_DELAY);
    gpio_set_level(level_sensor_obj->rs485_dir_pin, 0);
    printf("sent data sucessfully");

    return SYSTEM_OK;
 }
error_type_t rs485_read(level_sensor_t* level_sensor_obj, char* data, size_t buffer_size, size_t* read_size){
    if (level_sensor_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    
    if (!level_sensor_obj->activate)
    {
        return SYSTEM_INVALID_STATE;
    }
    
    *read_size = uart_read_bytes(level_sensor_obj->uart_num, data, buffer_size, pdMS_TO_TICKS(200));
    if (*read_size < buffer_size)
    {
        data[*read_size] = '\0';
        printf("received data sucessfully\n");

         
    }

      return SYSTEM_OK;
 }

error_type_t level_sensor_deinit(level_sensor_t* level_sensor_obj){
    if (level_sensor_obj == NULL)return SYSTEM_NULL_PARAMETER;
    level_sensor_obj->activate = false;
    return SYSTEM_OK;
    
}

error_type_t level_sensor_destroy(level_sensor_t** level_sensor_obj){
    if (*level_sensor_obj == NULL) return SYSTEM_NULL_PARAMETER;
    free(*level_sensor_obj);
    *level_sensor_obj = NULL;

    return SYSTEM_OK;   
}


