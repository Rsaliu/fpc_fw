#include <level_sensor.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


struct level_sensor_t
{
    int id; 
    rs485_config_t rs485;
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
    if(!level_sensor_obj->activate)return SYSTEM_INVALID_STATE;
    level_sensor_obj->activate = true;
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


