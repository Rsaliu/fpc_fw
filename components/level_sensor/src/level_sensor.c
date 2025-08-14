#include <level_sensor.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


struct level_sensor_t
{
    int id; 
    rs485_t* rs485;
    bool activate;
};

level_sensor_t* level_sensor_create(const level_sensor_config_t* config){
    if (config == NULL)
    {
        return NULL;
    }

    level_sensor_t* level_sensor_obj = (level_sensor_t*)malloc(sizeof(level_sensor_t));
    level_sensor_obj->id = config->id;
    level_sensor_obj->rs485 = config->rs485;
    level_sensor_obj->activate = false;

    return level_sensor_obj;   
}

error_type_t level_sensor_init(level_sensor_t* level_sensor_obj){
    if(level_sensor_obj == NULL)return SYSTEM_NULL_PARAMETER;
    if(level_sensor_obj->activate)return SYSTEM_INVALID_STATE;
    level_sensor_obj->activate = true;
    return SYSTEM_OK;
}

error_type_t level_sensor_write(level_sensor_t* level_sensor_obj){
    if( level_sensor_obj == NULL){
        return SYSTEM_NULL_PARAMETER;
    }

 return SYSTEM_OK;
}

error_type_t level_sensor_read(level_sensor_t* level_sensor_obj, uint16_t* level){
    return SYSTEM_OK;
}


error_type_t level_sensor_deinit(level_sensor_t* level_sensor_obj){
    if (level_sensor_obj == NULL)return SYSTEM_NULL_PARAMETER;
    level_sensor_obj->activate = false;
    return SYSTEM_OK;
    
}

error_type_t level_sensor_destroy(level_sensor_t** level_sensor_obj){
    if (*level_sensor_obj == NULL) return SYSTEM_NULL_PARAMETER;
    if ((*level_sensor_obj)->activate)
    {
        level_sensor_deinit(*level_sensor_obj);
    }
    
    free(*level_sensor_obj);
    *level_sensor_obj = NULL;

    return SYSTEM_OK;   
}

level_sensor_interface_t string_to_level_sensor_interface(const char* interface_str){
    if (strcmp(interface_str, "RS485") == 0)
    {
        return LEVEL_SENSOR_INTERFACE_RS485;
    }else if (strcmp(interface_str, "UART") == 0)
    {
        return LEVEL_SENSOR_INTERFACE_UART;
    }else if (strcmp(interface_str, "PWM") == 0)
    {
        return LEVEL_SENSOR_INTERFACE_PWM;
    }else
    {
        return LEVEL_SENSOR_INTERFACE_RS485;
    }
       
}

level_sensor_protocol_t string_to_level_sensor_protocol(const char* protocol_str){
    if (strcmp(protocol_str, "GL_A01_PROTOCOL") == 0)
    {
        return GL_A01_PROTOCOL;
    }else
    {
        printf("Unknow\n");
        return GL_A01_PROTOCOL;
    }
}


