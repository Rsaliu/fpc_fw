#ifndef __RS485_CONTEXT_LIBRARY_H__
#define __RS485_CONTEXT_LIBRARY_H__
#include <common_headers.h>
#include <stdint.h>
#include <stddef.h>

error_type_t rs485_context_send_receive(void*context, uint8_t* send_buff, int send_buff_size, uint8_t* receive_buff, int* receive_buff_size);

 
#endif