#ifndef __PROTOCOL_LIBRARY_H__
#define __PROTOCOL_LIBRARY_H__
#include <common_headers.h>
#include <stdint.h>

error_type_t protocol_gl_a01_write_address(uint8_t current_addr, uint8_t new_addr, uint8_t* buffer, uint8_t buff_size);
error_type_t protocol_gl_a01_read_level(uint8_t slave_addr,uint8_t* buffer,  int buff_size, uint8_t* payload);
error_type_t protocol_gl_a01_read_temp(uint8_t slave_addr,uint8_t* temp_buffer, uint8_t buff_size);
error_type_t protocol_gl_a01_interpreter(uint8_t* buffer, int buffer_Size,int* read_level );

#endif
