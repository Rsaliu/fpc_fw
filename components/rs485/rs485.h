#ifndef __RS485_LIBRARY_H__
#define __RS485_LIBRARY_H__
#include <common_headers.h>
#include <stdint.h>
#include <stddef.h>
typedef struct rs485_t rs485_t;



typedef struct 
{
    int uart_num;
    int rs485_di_pin; //driver input goes to tx pin 
    int rs485_ro_pin; // receiver output goes to rx pin
    int rs485_dir_pin; // de/re for bidirectional communication.
    int baud_rate;
   
    
}rs485_config_t;


rs485_t* rs485_create(const rs485_config_t* config);
error_type_t rs485_init(rs485_t* rs485_obj);
error_type_t rs485_write(rs485_t* rs485_obj, const char* data, size_t buffer_size);
error_type_t rs485_read(rs485_t* rs485_obj, char* data, size_t buffer_size, int* read_size);
error_type_t rs485_destroy(rs485_t** rs485_obj);
error_type_t rs485_deinit(rs485_t* rs485_obj);

#endif
