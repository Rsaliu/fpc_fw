#include <protocol.h>
#include <crc.h> 
#include <stdint.h>
#include <string.h>
#include <stdio.h>

error_type_t protocol_gl_a01_write_address(uint8_t slave_addr, uint8_t buff_size, uint8_t* buffer){
      if (buff_size!= 8)
    {
        return SYSTEM_INVALID_PARAMETER;
    }
    buffer[0] = slave_addr;
    buffer[1] = 0x06;
    buffer[2] = 0x02;
    buffer[3] = 0x00;
    buffer[4] = 0x00;
    buffer[5] = 0x05;
    uint16_t crc = MODBUS_CRC16_v3(buffer,6);
    buffer[6] = crc & 0x0FF;
    buffer[7] = (crc >> 8) & 0x0FF; 


    return SYSTEM_OK;

}
                                        
error_type_t protocol_gl_a01_read_level(uint8_t slave_addr, uint8_t* buffer,int buff_size, uint8_t* payload_size){
    if (buff_size < 8)
    {
        return SYSTEM_INVALID_PARAMETER;
    }
    buffer[0] = slave_addr;
    buffer[1]= 0x03; // write address
    buffer[2]= 0x01; // register upper bit address
    buffer[3]= 0x00; // register lower bit address
    buffer[4]= 0x00; // data upper bit address
    buffer[5]= 0x01; // data lower bit address

    // crc calculation
    uint16_t crc = MODBUS_CRC16_v3(buffer,6);
    buffer[6]= crc & 0x0FF; //crc upper bit address
    buffer[7] = (crc >> 8) & 0x0FF; // crc lower bit address
    *payload_size = 8;
    return SYSTEM_OK;
}

error_type_t protocol_gl_a01_read_temp(uint8_t slave_addr, uint8_t buff_size, uint8_t* temp_buffer){
    if (buff_size != 8)
    {
        return SYSTEM_INVALID_PARAMETER;
    }

    temp_buffer[0] = slave_addr;
    temp_buffer[1] = 0x03;
    temp_buffer[2] = 0x01;
    temp_buffer[3] = 0x02;
    temp_buffer[4] = 0x00;
    temp_buffer[5] = 0x01;
    uint16_t crc = MODBUS_CRC16_v3(temp_buffer,6);
    temp_buffer[6] = crc & 0x0FF;
    temp_buffer[7] = (crc >> 8) & 0x0FF;
    
    return SYSTEM_OK;
    
}

error_type_t protocol_gl_a01_interpreter( uint8_t* buffer, int buffer_Size, uint16_t* sensor_data){

    uint16_t calculated_crc = MODBUS_CRC16_v3(buffer,buffer_Size-2);
    printf("calculated crc: %x\n", calculated_crc);
    //compare the claculated_crc and actual crc
    uint16_t received_crc = buffer[buffer_Size-2]| (buffer[buffer_Size-1] << 8);
    printf("received crc: %x\n", received_crc);

    if (received_crc != calculated_crc)
    {
        printf("CRC does not correspond");           
        return SYSTEM_INVALID_PARAMETER;
    }
    *sensor_data = (buffer[3] << 8) | buffer[4];
    printf("Tank level: %umm\n", *sensor_data);
    
    return SYSTEM_OK;
}