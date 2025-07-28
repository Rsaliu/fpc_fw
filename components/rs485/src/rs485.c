#include <rs485.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#define BUF_SIZE    (127)



struct rs485_t
{
    int uart_num;
    int rs485_di_pin; 
    int rs485_ro_pin;
    int rs485_dir_pin;
    int baud_rate;
    bool activate;
};

rs485_t* rs485_create(const rs485_config_t* config){
    if (config == NULL)
    {
        return NULL;
    }

    rs485_t* rs485_obj = (rs485_t*)malloc(sizeof(rs485_t));
    rs485_obj->uart_num = config->uart_num;
    rs485_obj->rs485_ro_pin = config->rs485_ro_pin;
    rs485_obj->rs485_di_pin = config->rs485_di_pin;
    rs485_obj->rs485_dir_pin = config->rs485_dir_pin;
    rs485_obj->baud_rate = config->baud_rate;
    rs485_obj->activate = false;

    return rs485_obj;   
}

error_type_t rs485_init( rs485_t* rs485_obj){
    if(rs485_obj == NULL)return SYSTEM_NULL_PARAMETER;

    if (rs485_obj->uart_num < 0) return SYSTEM_INVALID_PARAMETER;

    if(rs485_obj->rs485_di_pin < 0|| rs485_obj->rs485_ro_pin < 0 || rs485_obj->rs485_dir_pin < 0){
        return SYSTEM_INVALID_PIN_NUMBER;
    }

    if(rs485_obj->baud_rate < 0)return SYSTEM_INVALID_BAUD_RATE;
    

    uart_config_t uart_config = {
    .baud_rate = rs485_obj->baud_rate,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_APB,
   };

    // configure uart pin

    uart_param_config(rs485_obj->uart_num, &uart_config);
    uart_set_pin(rs485_obj->uart_num, rs485_obj->rs485_di_pin ,rs485_obj->rs485_ro_pin, rs485_obj->rs485_dir_pin  , UART_PIN_NO_CHANGE );
    uart_driver_install(rs485_obj->uart_num, BUF_SIZE*2,0,0,NULL,0);

    //configure gpio pin
    //gpio_set_direction(rs485_obj->rs485_dir_pin, GPIO_MODE_OUTPUT);

    //gpio_set_level(rs485_obj->rs485_dir_pin, 0); // 0 -> LOW, 1 -> HIGH
                                      // set the rs485 to receiver mode    
     uart_set_mode(rs485_obj->uart_num, UART_MODE_RS485_HALF_DUPLEX);                                 

    rs485_obj->activate = true;

    return SYSTEM_OK;
}

error_type_t rs485_write( rs485_t* rs485_obj, const char* data, size_t buffer_size){
    if (rs485_obj == NULL)
    {
        printf(" rs485 is returing null\n");
        return SYSTEM_NULL_PARAMETER;
    }

    if(!rs485_obj->activate){
        printf(" rs485 is returing invalid state\n");
        return SYSTEM_INVALID_STATE;
    }
    //gpio_set_level(rs485_obj->rs485_dir_pin, 1);
    uart_write_bytes(rs485_obj->uart_num, data, buffer_size);
    uart_wait_tx_done(rs485_obj->uart_num, portMAX_DELAY);
    //gpio_set_level(rs485_obj->rs485_dir_pin, 0);
    printf("sent data sucessfully");

    return SYSTEM_OK;
 }
error_type_t rs485_read( rs485_t* rs485_obj, char* data, size_t buffer_size, int* read_size){
    if (rs485_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    
    *read_size = uart_read_bytes(rs485_obj->uart_num, data, buffer_size, -1); 
    if (*read_size != -1)
    {
         printf("received data sucessfully\n");
    }

    printf("read size: %d\n", *read_size);      
      return SYSTEM_OK;
 }

error_type_t rs485_deinit( rs485_t* rs485_obj){
    if (rs485_obj == NULL)return SYSTEM_NULL_PARAMETER;
    rs485_obj->activate = false;
    uart_driver_delete(rs485_obj->uart_num);
    return SYSTEM_OK;
    
}

error_type_t rs485_destroy( rs485_t** rs485_obj){
    if (*rs485_obj == NULL) return SYSTEM_NULL_PARAMETER;
    if ((*rs485_obj)->activate)
    {
        rs485_deinit(*rs485_obj);
    }
    
    free(*rs485_obj);
    *rs485_obj = NULL;

    return SYSTEM_OK;   
}


