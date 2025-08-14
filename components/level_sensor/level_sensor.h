#ifndef __LEVEL_SENSOR_LIBRARY_H__
#define __LEVEL_SENSOR_LIBRARY_H__
#include <common_headers.h>
#include <stdint.h>
#include <stddef.h>
#include <rs485.h>
typedef struct level_sensor_t level_sensor_t;

typedef enum {
    LEVEL_SENSOR_INTERFACE_RS485,
    LEVEL_SENSOR_INTERFACE_UART,
    LEVEL_SENSOR_INTERFACE_PWM

}level_sensor_interface_t;

typedef enum {
    GL_A01_PROTOCOL

}level_sensor_protocol_t;

typedef struct 
{
    int id;
    uint8_t sensor_addr;   
    rs485_t* rs485;
    level_sensor_interface_t interface;
    level_sensor_protocol_t level_sensor_protocol;
}level_sensor_config_t;


level_sensor_t* level_sensor_create(const level_sensor_config_t* config);
error_type_t level_sensor_init(level_sensor_t* level_sensor_obj);
error_type_t level_sensor_write(level_sensor_t* level_sensor_obj);
error_type_t level_sensor_read(level_sensor_t* level_sensor_obj, uint16_t* level);
error_type_t level_sensor_destroy(level_sensor_t** level_sensor_obj);
error_type_t level_sensor_deinit(level_sensor_t* level_sensor_obj);

level_sensor_interface_t string_to_level_sensor_interface(const char* interface_str);
level_sensor_protocol_t string_to_level_sensor_protocol(const char* protocol_str);
#endif
