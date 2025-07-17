#ifndef __LEVEL_SENSOR_LIBRARY_H__
#define __LEVEL_SENSOR_LIBRARY_H__
#include <common_headers.h>
#include <stdint.h>
#include <stddef.h>
typedef struct level_sensor_t level_sensor_t;

typedef error_type_t (*protocol_callback_t)(uint8_t addr,uint8_t buf_size, uint8_t* buffer, uint8_t* payload_size);
typedef error_type_t(*transport_medium_t)(uint8_t* buffer, uint8_t buff_size);

 
typedef struct 
{
    int id;   
    uint8_t sensor_addr;
    protocol_callback_t protocol;
    transport_medium_t send;
}level_sensor_config_t;


level_sensor_t* level_sensor_create(const level_sensor_config_t* config);
error_type_t level_sensor_init(level_sensor_t* level_sensor_obj);
error_type_t level_sensor_read(level_sensor_t* level_sensor_obj);
error_type_t level_sensor_destroy(level_sensor_t** level_sensor_obj);
error_type_t level_sensor_deinit(level_sensor_t* level_sensor_obj);

#endif
