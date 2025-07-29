#include <level_sensor.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct level_sensor_t
{
    level_sensor_config_t *config;
    bool activate;
};
static const int sensor_buffer_size = 50;
static const int receive_buff_size = 100;

level_sensor_t *level_sensor_create(level_sensor_config_t config)
{

    level_sensor_t *level_sensor_obj = (level_sensor_t *)malloc(sizeof(level_sensor_t));
    if (!level_sensor_obj)
        return NULL;

    level_sensor_obj->config = (level_sensor_config_t *)malloc(sizeof(level_sensor_config_t));
    if (level_sensor_obj->config == NULL)
    {
        return NULL;
    }

    memcpy(level_sensor_obj->config, &config, sizeof(level_sensor_config_t));
    level_sensor_obj->activate = false;

    return level_sensor_obj;
}

error_type_t level_sensor_init(level_sensor_t *level_sensor_obj)
{
    if (level_sensor_obj == NULL || level_sensor_obj->config == NULL)
        return SYSTEM_NULL_PARAMETER;
    if (level_sensor_obj->activate)
        return SYSTEM_INVALID_STATE;
    level_sensor_obj->activate = true;
    return SYSTEM_OK;
}

error_type_t level_sensor_read(level_sensor_t *level_sensor_obj, uint16_t* read_level)
{
    if (level_sensor_obj == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    if (!level_sensor_obj->activate)
        return SYSTEM_INVALID_STATE;
    uint8_t buffer[sensor_buffer_size];
    uint8_t payload_size = 0; // store the amount of send data
    uint8_t receive_buff[receive_buff_size];
    int receive_payload = 0; // store the amount of receive data
    error_type_t err;

    printf("Calling protocol()...\n");
    err = level_sensor_obj->config->protocol(level_sensor_obj->config->sensor_addr, buffer, sensor_buffer_size, &payload_size);
    printf("Protocol returned %d, payload_size=%d\n", err, payload_size);
    if (err != SYSTEM_OK)
    {
        printf("failed to run protocol");
        return SYSTEM_INVALID_PARAMETER;
    }
    if (payload_size == 0)
    {

        printf("payload return invalid lenght \n");
        return SYSTEM_INVALID_LENGTH;
    }

    for (int x = 0; x < payload_size; x++)
    {
        printf("payload[%d]: %x\n", x, buffer[x]);
    }
    err = level_sensor_obj->config->send_recive(level_sensor_obj->config->medium_context, buffer, payload_size, receive_buff, &receive_payload);
    if (err != SYSTEM_OK)
    {
        return SYSTEM_INVALID_PARAMETER;
    }
    if (receive_payload == 0)
    {
        printf("receive payload return invalid lenght\n");
        return SYSTEM_INVALID_LENGTH;
    }

    for (int x = 0; x < receive_payload; x++)
    {
        printf("received[%d]: %x\n", x, receive_buff[x]);
    }
    err = level_sensor_obj->config->interpreter(receive_buff, receive_payload, read_level);
    if (err != SYSTEM_OK)
    {
        printf("failed to interepret protocol\n");
        return SYSTEM_INVALID_PARAMETER;
    }

    return SYSTEM_OK;
}

error_type_t level_sensor_deinit(level_sensor_t *level_sensor_obj)
{
    if (level_sensor_obj == NULL)
        return SYSTEM_NULL_PARAMETER;
    level_sensor_obj->activate = false;
    return SYSTEM_OK;
}

error_type_t level_sensor_destroy(level_sensor_t **level_sensor_obj)
{
    if (*level_sensor_obj == NULL)
        return SYSTEM_NULL_PARAMETER;
    if ((*level_sensor_obj)->activate)
    {
        level_sensor_deinit(*level_sensor_obj);
    }

    free(*level_sensor_obj);
    *level_sensor_obj = NULL;

    return SYSTEM_OK;
}
