#include <level_sensor.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <protocol.h>


struct level_sensor_t
{
    int id; 
    uint8_t sensor_addr;
    protocol_callback_t protocol;
    transport_medium_t send;
    bool activate;
};
static const int sensor_buffer_size = 50;

level_sensor_t* level_sensor_create(const level_sensor_config_t* config){
    if (config == NULL)
    {
        return NULL;
    }

    level_sensor_t* level_sensor_obj = (level_sensor_t*)malloc(sizeof(level_sensor_t));
    level_sensor_obj->id = config->id;
    level_sensor_obj->sensor_addr = config->sensor_addr;
    level_sensor_obj->protocol = config->protocol;
    level_sensor_obj->send = config->send;
    level_sensor_obj->activate = false;

    return level_sensor_obj;   
}

error_type_t level_sensor_init(level_sensor_t* level_sensor_obj){
    if(level_sensor_obj == NULL)return SYSTEM_NULL_PARAMETER;
    if(level_sensor_obj->activate)return SYSTEM_INVALID_STATE;
    level_sensor_obj->activate = true;
    return SYSTEM_OK;
}

error_type_t level_sensor_read(level_sensor_t* level_sensor_obj){
    if (level_sensor_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    uint8_t buffer[sensor_buffer_size];
    uint8_t payload_size = 0;
    
    level_sensor_obj->protocol(level_sensor_obj->sensor_addr,sensor_buffer_size,buffer,&payload_size);
    level_sensor_obj->send(buffer, payload_size);
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


