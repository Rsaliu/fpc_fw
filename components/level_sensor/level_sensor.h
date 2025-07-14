#ifndef __LEVEL_SENSOR_LIBRARY_H__
#define __LEVEL_SENSOR_LIBRARY_H__
#include <common_headers.h>
#include <stdint.h>
#include <stddef.h>
typedef struct level_sensor_t level_sensor_t;



typedef struct 
{
    int uart_num;
    int rs485_di_pin; //driver input goes to tx pin 
    int rs485_ro_pin; // receiver output goes to rx pin
    int rs485_dir_pin; // de/re for bidirectional communication.
    int baud_rate;
   
    
}level_sensor_config_t;


level_sensor_t* level_sensor_create(const level_sensor_config_t* config);
error_type_t level_sensor_init(level_sensor_t* level_sensor_obj);
error_type_t rs485_write(level_sensor_t* level_sensor_obj, const char* data, size_t buffer_size);
error_type_t rs485_read(level_sensor_t* level_sensor_obj, char* data, size_t buffer_size, int* read_size);
error_type_t level_sensor_read(level_sensor_t* level_sensor_obj);
error_type_t level_sensor_destroy(level_sensor_t** level_sensor_obj);
error_type_t level_sensor_deinit(level_sensor_t* level_sensor_obj);

#endif
