#include <rs485_context.h>
#include <rs485.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"

const int send_buffer_size = 7;
static const char* TAG = "RS485_CONTEXT";
error_type_t rs485_context_send_receive(void *context, uint8_t *send_buff, int send_buff_size,
                                        uint8_t *receive_buff, int *receive_buff_size)
{
    if (send_buff_size == 0 || receive_buff_size == 0)
    {
        return SYSTEM_INVALID_PARAMETER;
    }
    if (context == NULL || send_buff == NULL || receive_buff == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    

    rs485_t *sender = (rs485_t*)context;
    error_type_t err;
    err = rs485_write(sender, (char *)send_buff, send_buff_size);
    if (err != SYSTEM_OK)
    {
        ESP_LOGE(TAG,"rs485 write failed!!!\n");
        ESP_LOGE(TAG,"error code: %d\n", err);
        return SYSTEM_INVALID_PARAMETER;
    }
    
    send_buff_size = send_buffer_size;
    err = rs485_read(sender, (char *)receive_buff, send_buff_size, receive_buff_size);
    if (err != SYSTEM_OK)
    {
        ESP_LOGE(TAG,"rs485 read failed!!! \n");
        return SYSTEM_INVALID_PARAMETER;
    }


    return SYSTEM_OK;
}