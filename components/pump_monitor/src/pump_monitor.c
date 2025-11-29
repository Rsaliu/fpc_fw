#include "pump_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

static const char *TAG = "pump_monitor";

pump_monitor_t *pump_monitor_create(pump_monitor_config_t config)
{
    pump_monitor_t *pump_monitor = (pump_monitor_t *)malloc(sizeof(pump_monitor_t));
    if (pump_monitor == NULL)
    {
        return NULL; 
    }

    memset(pump_monitor, 0, sizeof(pump_monitor_t));

    pump_monitor->config = (pump_monitor_config_t *)malloc(sizeof(pump_monitor_config_t));
    if (pump_monitor->config == NULL)
    {
        free(pump_monitor);
        return NULL;
    }

    memcpy(pump_monitor->config, &config, sizeof(pump_monitor_config_t));
    pump_monitor->state = PUMP_MONITOR_NOT_INITIALIZED;
    pump_monitor->pump_state = PUMP_STATE_MACHINE_NORMAL_STATE;
    pump_monitor->subscriber_count = 0;                         

    for (int i = 0; i < PUMP_MONITOR_MAXIMUM_SUBSCRIBER; ++i)
    {
        pump_monitor->subscribers[i].id = -1;
        pump_monitor->subscribers[i].in_use = false;
        
    }

    return pump_monitor;
}

error_type_t pump_monitor_init(pump_monitor_t *pump_monitor)
{
    if (pump_monitor == NULL || pump_monitor->config == NULL)
    {
        return SYSTEM_NULL_PARAMETER; 
    }
    
    if (pump_monitor->config->pump == NULL || pump_monitor->config->sensor == NULL)
    {
        return SYSTEM_INVALID_PARAMETER; 
    }
    if (pump_monitor->state != PUMP_MONITOR_NOT_INITIALIZED)
    {
        return SYSTEM_INVALID_STATE; 
    }

    pump_monitor->state = PUMP_MONITOR_INITIALIZED;
    ESP_LOGI(TAG, "Pump monitor initialized");
    return SYSTEM_OK;
}

error_type_t pump_monitor_deinit(pump_monitor_t *pump_monitor)
{
    if (pump_monitor == NULL || pump_monitor->config == NULL)
    {
        return SYSTEM_NULL_PARAMETER; 
    }

    if (pump_monitor->state == PUMP_MONITOR_NOT_INITIALIZED)
    {
        return SYSTEM_INVALID_STATE; 
    }

    pump_monitor->state = PUMP_MONITOR_NOT_INITIALIZED;
    ESP_LOGI(TAG, "Pump monitor deinitialized");
    return SYSTEM_OK;
}

error_type_t pump_monitor_destroy(pump_monitor_t **pump_monitor)
{
    if (pump_monitor == NULL || *pump_monitor == NULL)
    {
        return SYSTEM_NULL_PARAMETER; 
    }

    if ((*pump_monitor)->config != NULL)
    {
        free((*pump_monitor)->config);
    }

    free(*pump_monitor);
    *pump_monitor = NULL; 

    ESP_LOGI(TAG, "Pump monitor destroyed");
    return SYSTEM_OK;
}

error_type_t pump_monitor_get_state(const pump_monitor_t *pump_monitor, pump_monitor_state_t *state)
{
    if (pump_monitor == NULL || state == NULL)
    {
        return SYSTEM_NULL_PARAMETER; 
    }

    *state = pump_monitor->state;
    return SYSTEM_OK;
}

error_type_t pump_monitor_get_config(const pump_monitor_t *pump_monitor, pump_monitor_config_t *config)
{
    if (pump_monitor == NULL || config == NULL)
    {
        return SYSTEM_NULL_PARAMETER; 
    }

    memcpy(config, pump_monitor->config, sizeof(pump_monitor_config_t));
    return SYSTEM_OK;
}

static event_type_t state_machine_state_to_event(pump_state_machine_state_t state)
{
    switch (state)
    {
    case PUMP_STATE_MACHINE_NORMAL_STATE:
        return EVENT_PUMP_NORMAL; 

    case PUMP_STATE_MACHINE_UNDERCURRENT_STATE:
        return EVENT_PUMP_UNDERCURRENT; 

    case PUMP_STATE_MACHINE_OVERCURRENT_STATE:
        return EVENT_PUMP_OVERCURRENT; 

    default:
        return EVENT_UNKNOWN;
    }
}

error_type_t pump_monitor_check_current(pump_monitor_t *pump_monitor)
{
    if (pump_monitor == NULL || pump_monitor->config == NULL || pump_monitor->config->sensor == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    float current_amp_reading = 0.0f;
    error_type_t err = current_sensor_get_current(pump_monitor->config->sensor, &current_amp_reading);
    if (err != SYSTEM_OK)
    {
        return err;
    }

    pump_config_t pump_config;
    err = pump_get_config(pump_monitor->config->pump, &pump_config);
    if (err != SYSTEM_OK)
    {
        return err;
    }

    float normal_current_supply = pump_config.current_rating;
    float low_current_supply = normal_current_supply - normal_current_supply * 0.05f;
    float high_current_supply = normal_current_supply + normal_current_supply * 0.05f;

    pump_state_machine_state_t previous_state = pump_monitor->pump_state;

    switch (pump_monitor->pump_state)
    {

    case PUMP_STATE_MACHINE_NORMAL_STATE:
        if (current_amp_reading < low_current_supply)
        {
            pump_monitor->pump_state = PUMP_STATE_MACHINE_UNDERCURRENT_STATE;

            ESP_LOGI(TAG, "Current supply is below normal: %.2f < %.2f", current_amp_reading, low_current_supply);
        }
        else if (current_amp_reading > high_current_supply)
        {
            pump_monitor->pump_state = PUMP_STATE_MACHINE_OVERCURRENT_STATE;

            ESP_LOGI(TAG, "Current supply is above normal: %.2f > %.2f", current_amp_reading, high_current_supply);
        }
        break;

    case PUMP_STATE_MACHINE_UNDERCURRENT_STATE:
        if (current_amp_reading >= low_current_supply && current_amp_reading <= high_current_supply)
        {
            pump_monitor->pump_state = PUMP_STATE_MACHINE_NORMAL_STATE;
            ESP_LOGI(TAG, "Current supply is normal: %.2f", current_amp_reading);
        }
        break;

    case PUMP_STATE_MACHINE_OVERCURRENT_STATE:
        if (current_amp_reading >= low_current_supply && current_amp_reading <= high_current_supply)
        {
            pump_monitor->pump_state = PUMP_STATE_MACHINE_NORMAL_STATE;
            ESP_LOGI(TAG, "Current supply is normal: %.2f", current_amp_reading);
        }
        break;

    default:
        return SYSTEM_INVALID_STATE;
    }

    if (pump_monitor->pump_state != previous_state)
    {
        for (int i = 0; i < PUMP_MONITOR_MAXIMUM_SUBSCRIBER; ++i)
        {
            if (pump_monitor->subscribers[i].in_use)
            {
                pump_monitor_event_hook_t *hook = &pump_monitor->subscribers[i].hook;
                if (hook && hook->callback) 
                {
                    hook->callback(hook->context,
                                   hook->actuator_id,
                                   state_machine_state_to_event(pump_monitor->pump_state),
                                   pump_monitor->config->id);
                }
                else
                {
                    ESP_LOGW(TAG, "Skipping subscriber %d - no callback assigned", pump_monitor->subscribers[i].id);
                }
            }
        }
    }

    return SYSTEM_OK;
}

error_type_t pump_monitor_subscribe_event(pump_monitor_t *pump_monitor, const pump_monitor_event_hook_t *hook, int *event_id)
{
    if (pump_monitor == NULL || hook == NULL || event_id == NULL)
    {
        ESP_LOGE(TAG, "Null parameter in pump_monitor_subscribe_event");
        return SYSTEM_NULL_PARAMETER;
    }

    if (pump_monitor->state != PUMP_MONITOR_INITIALIZED)
    {
        ESP_LOGE(TAG, "Pump monitor is not initialized");
        return SYSTEM_INVALID_STATE;
    }

    if (pump_monitor->subscriber_count >= PUMP_MONITOR_MAXIMUM_SUBSCRIBER)
    {
        return SYSTEM_BUFFER_OVERFLOW;
    }

    /* find first free slot */
    int slot = -1;
    for (int i = 0; i < PUMP_MONITOR_MAXIMUM_SUBSCRIBER; ++i)
    {
        if (!pump_monitor->subscribers[i].in_use) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        return SYSTEM_BUFFER_OVERFLOW;
    }

    /* copy hook inline (safe) */
    pump_monitor->subscribers[slot].hook = *hook;
    pump_monitor->subscribers[slot].id = slot;
    pump_monitor->subscribers[slot].in_use = true;
    pump_monitor->subscriber_count++;

    *event_id = pump_monitor->subscribers[slot].id;

    ESP_LOGI(TAG, "Subscribed event id=%d (slot=%d)", *event_id, slot);
    return SYSTEM_OK;
}

error_type_t pump_monitor_unsubscribe_event(pump_monitor_t *pump_monitor, int event_id)
{
    if (pump_monitor == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    if (event_id < 0 || event_id >= PUMP_MONITOR_MAXIMUM_SUBSCRIBER) {
        return SYSTEM_INVALID_PARAMETER;
    }

    if (pump_monitor->state != PUMP_MONITOR_INITIALIZED)
    {
        return SYSTEM_INVALID_STATE;
    }

    if (!pump_monitor->subscribers[event_id].in_use) {
        return SYSTEM_INVALID_PARAMETER; 
    }

    /* clear the slot */
    pump_monitor->subscribers[event_id].in_use = false;
    pump_monitor->subscribers[event_id].id = -1;
    memset(&pump_monitor->subscribers[event_id].hook, 0, sizeof(pump_monitor_event_hook_t));
    pump_monitor->subscriber_count--;

    ESP_LOGI(TAG, "Unsubscribed event id=%d", event_id);

    return SYSTEM_OK;
}
