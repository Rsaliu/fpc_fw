#ifndef __LEVEL_SENSOR_LIBRARY_H__
#define __LEVEL_SENSOR_LIBRARY_H__
#include <common_headers.h>
#include <stdint.h>
#include <stddef.h>
#include <rs485.h>
typedef struct level_sensor_t level_sensor_t;



typedef struct 
{
    int id;   
    rs485_t* rs485;
}level_sensor_config_t;


level_sensor_t* level_sensor_create(const level_sensor_config_t* config);
error_type_t level_sensor_init(level_sensor_t* level_sensor_obj);
error_type_t level_sensor_write(level_sensor_t* level_sensor_obj);
error_type_t level_sensor_read(level_sensor_t* level_sensor_obj);
error_type_t level_sensor_destroy(level_sensor_t** level_sensor_obj);
error_type_t level_sensor_deinit(level_sensor_t* level_sensor_obj);

#endif
