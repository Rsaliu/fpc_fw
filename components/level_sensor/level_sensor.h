#ifndef __LEVEL_SENSOR_LIBRARY_H__
#define __LEVEL_SENSOR_LIBRARY_H__
#include <common_headers.h>
#include <stdint.h>
#include <stddef.h>
typedef struct level_sensor_t level_sensor_t;

typedef error_type_t (*protocol_callback_t)( uint8_t slave_addr, uint8_t* buffer, int buff_size, uint8_t* payload_size);
typedef error_type_t(*send_receive_t)(void* context, uint8_t* send_buff, int send_buff_size, uint8_t* receive_buff, int* receive_buff_size);
typedef error_type_t(*protocol_interpreter_t)(uint8_t* buffer, int buff_size, uint16_t* sensor_data);
 
typedef struct 
{
    int id;   
    uint8_t sensor_addr;
    protocol_callback_t protocol;
    void* medium_context;
    send_receive_t send_recive;
    protocol_interpreter_t interpreter;
}level_sensor_config_t;


level_sensor_t* level_sensor_create(level_sensor_config_t config);
error_type_t level_sensor_init(level_sensor_t* level_sensor_obj);
error_type_t level_sensor_read(level_sensor_t* level_sensor_obj, uint16_t* read_level);
error_type_t level_sensor_destroy(level_sensor_t** level_sensor_obj);
error_type_t level_sensor_deinit(level_sensor_t* level_sensor_obj);

#endif
