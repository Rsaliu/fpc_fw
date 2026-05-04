#include "pump_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <math.h>

static const char *TAG = "pump_monitor";
#define OVERCURRENT_EVENT_QUEUE_LENGTH 10
#define PUMP_MONITOR_MAXIMUM_SUBSCRIBER 10
#define PUMP_MONITOR_MAX_SAMPLE_SIZE 50 // Maximum number of samples for averaging current readings, can be adjusted based on requirements




struct pump_monitor_t
{
    pump_monitor_config_t config;                                           // Pointer to the pump monitor configuration
    float sampled_current_values[PUMP_MONITOR_MAX_SAMPLE_SIZE]; // Buffer to hold sampled current values for averaging (if applicable)
    pump_monitor_state_t state;                                              // State of the pump monitor
    pump_state_machine_state_t state_machine_state;                            // State of the pump monitor state machine
    pump_monitor_subscriber_t subscribers[PUMP_MONITOR_MAXIMUM_SUBSCRIBER];   // subscriber slots (inline)
    int subscriber_count;                                                    // Count of subscribers
    //SemaphoreHandle_t read_mutex;                                           // Mutex for reading operations
    //bool value_ready;                                                       // Flag indicating if a new value is ready
    measurement_item_t measurement_item;                                          // Measurement item for continuous reading
};

pump_monitor_t *pump_monitor_create(pump_monitor_config_t config)
{
    pump_monitor_t *pump_monitor = (pump_monitor_t *)malloc(sizeof(pump_monitor_t));
    if (pump_monitor == NULL)
    {
        return NULL; 
    }

    memcpy(&pump_monitor->config, &config, sizeof(pump_monitor_config_t));
    pump_monitor->state = PUMP_MONITOR_NOT_INITIALIZED;
    pump_monitor->subscriber_count = 0;  
    pump_monitor->state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;                       

    for (int i = 0; i < PUMP_MONITOR_MAXIMUM_SUBSCRIBER; ++i)
    {
        pump_monitor->subscribers[i].id = -1;
        pump_monitor->subscribers[i].in_use = false;
    }
    return pump_monitor;
}

error_type_t pump_monitor_init(pump_monitor_t *pump_monitor)
{
    if (pump_monitor == NULL || pump_monitor->config.pump == NULL || pump_monitor->config.sensor == NULL || pump_monitor->config.current_read_cb == NULL || pump_monitor->config.analytics_cb == NULL)
    {
        // log all the pointer values to help with debugging
        ESP_LOGE(TAG, "Null parameter in pump monitor configuration: pump_monitor=%p, pump=%p, sensor=%p, current_read_cb=%p, analytics_cb=%p", 
                 pump_monitor, pump_monitor->config.sensor, pump_monitor->config.pump, pump_monitor->config.current_read_cb, pump_monitor->config.analytics_cb);
        ESP_LOGE(TAG, "Null parameter in pump monitor configuration");
        return SYSTEM_NULL_PARAMETER; 
    }
    
    if (pump_monitor->config.number_of_samples_for_average <= 0 || pump_monitor->config.number_of_samples_for_average > PUMP_MONITOR_MAX_SAMPLE_SIZE)
    {
        ESP_LOGE(TAG, "Invalid pump monitor configuration");
        return SYSTEM_INVALID_PARAMETER; 
    }
    if (pump_monitor->state != PUMP_MONITOR_NOT_INITIALIZED)
    {
        return SYSTEM_INVALID_STATE; 
    }
    // // create mutex
    // pump_monitor->read_mutex = xSemaphoreCreateMutex();
    // if (pump_monitor->read_mutex == NULL)
    // {
    //     ESP_LOGE(TAG, "Failed to create read mutex");
    //     return SYSTEM_OPERATION_FAILED;
    // }

    pump_monitor->state = PUMP_MONITOR_INITIALIZED;
    ESP_LOGI(TAG, "Pump monitor initialized");
    return SYSTEM_OK;
}

error_type_t pump_monitor_deinit(pump_monitor_t *pump_monitor)
{
    if (pump_monitor == NULL)
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

    if ((*pump_monitor)->state != PUMP_MONITOR_NOT_INITIALIZED)
    {
        pump_monitor_deinit(*pump_monitor); 
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
    memcpy(state, &pump_monitor->state, sizeof(pump_monitor_state_t));
    return SYSTEM_OK;
}

error_type_t pump_monitor_get_config(const pump_monitor_t *pump_monitor, pump_monitor_config_t *config)
{
    if (pump_monitor == NULL || config == NULL)
    {
        return SYSTEM_NULL_PARAMETER; 
    }

    memcpy(config, &pump_monitor->config, sizeof(pump_monitor_config_t));
    return SYSTEM_OK;
}

static event_type_t state_machine_state_to_event(pump_state_machine_state_t state)
{
    ESP_LOGI(TAG, "Mapping state machine state %d to event", state);
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


// static error_type_t spool_event_queue_item(pump_monitor_t *pump_monitor)
// {
//     if (pump_monitor == NULL)
//     {
//         return SYSTEM_NULL_PARAMETER;
//     }
//      measurement_item_t item;
//     while(xQueueReceive(pump_monitor->event_queue, &item, pdMS_TO_TICKS(1000)) == pdTRUE){
//         uint16_t raw_value = 0;
//         error_type_t err = ads1115_read_conversion_register(item.context,item.channel, &raw_value);
//         if(err != SYSTEM_OK){
//             ESP_LOGI(TAG,"continuous measurement Error reading ADS1115 in callback: %d\n", err);
//             continue;
//         }
//         vTaskDelay(1 / portTICK_PERIOD_MS);
//         }
// }

error_type_t pump_monitor_check_current(pump_monitor_t *pump_monitor)
{
    if (pump_monitor == NULL || pump_monitor->config.sensor == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    if(pump_monitor->state != PUMP_MONITOR_INITIALIZED){
        return SYSTEM_INVALID_STATE;
    }
    // take read mutex
    // if (xSemaphoreTake(pump_monitor->read_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
    //     ESP_LOGE(TAG, "Failed to take read mutex");
    //     return SYSTEM_OPERATION_FAILED;
    // }
    // if (!pump_monitor->value_ready) {
    //     xSemaphoreGive(pump_monitor->read_mutex);
    //     ESP_LOGW(TAG, "No new value ready to read");
    //     return SYSTEM_BUSY;
    // }
    // item = pump_monitor->measurement_item;
    // pump_monitor->value_ready = false;
    // xSemaphoreGive(pump_monitor->read_mutex);
    current_value_read_callback_t current_value_reader = (current_value_read_callback_t)pump_monitor->config.current_read_cb;
    if(current_value_reader == NULL){
        ESP_LOGE(TAG, "Current value read callback not set");
        return SYSTEM_NULL_PARAMETER;
    }
    for(int i = 0; i < pump_monitor->config.number_of_samples_for_average; ++i){
        error_type_t err = current_value_reader((pump_monitor->config.sensor), &pump_monitor->sampled_current_values[i]);
        if (err != SYSTEM_OK)
        {
            ESP_LOGE(TAG, "Failed to read current value for sample %d: %d", i, err);
            return err;
        }
    }
    ESP_LOGI(TAG, "about to get config for pump in current check");
    pump_config_t pump_config;
    ESP_LOGI(TAG, "pointer to pump in config: %p", (pump_monitor->config.pump));
    error_type_t err = pump_get_config((pump_monitor->config.pump), &pump_config);
    if (err != SYSTEM_OK)
    {
        return err;
    }
    ESP_LOGI(TAG, "got pump config");
    float rated_current = pump_config.current_rating;
    float min_working_current = pump_config.min_working_current;


    pump_state_machine_state_t previous_state = pump_monitor->state_machine_state;
    if(!pump_monitor->config.analytics_cb){
        return SYSTEM_NULL_PARAMETER;
    }
    current_analytics_callback_t analytics_cb = (current_analytics_callback_t)pump_monitor->config.analytics_cb;
    err = analytics_cb(pump_monitor->sampled_current_values, pump_monitor->config.number_of_samples_for_average, rated_current, min_working_current, &pump_monitor->state_machine_state);
    if(err != SYSTEM_OK){
        ESP_LOGE(TAG, "Error in analytics callback: %d", err);
        return err;
    }
    if (pump_monitor->state_machine_state != previous_state)
    {
        ESP_LOGW(TAG, "Pump monitor state machine state changed from %d to %d", previous_state, pump_monitor->state_machine_state);
        for (int i = 0; i < PUMP_MONITOR_MAXIMUM_SUBSCRIBER; ++i)
        {
            if (pump_monitor->subscribers[i].in_use)
            {

                if(pump_monitor->subscribers[i].context == NULL){
                    ESP_LOGE(TAG, "Subscriber context is NULL for subscriber %d", pump_monitor->subscribers[i].id);
                    continue;
                }
                ESP_LOGI(TAG, "Notifying subscriber %d of state change to %d with context %p with callback %p", pump_monitor->subscribers[i].id, pump_monitor->state_machine_state, pump_monitor->subscribers[i].context, pump_monitor->subscribers[i].callback);
                if (pump_monitor->subscribers[i].callback) 
                {
                    pump_monitor->subscribers[i].callback((pump_monitor->subscribers[i].context),
                                   state_machine_state_to_event(pump_monitor->state_machine_state),
                                   pump_monitor->subscribers[i].id);
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


void update_overcurrent_subscribers(overcurrent_queue_item_t item)
{
    // pump_monitor_t *pump_monitor = (pump_monitor_t *)item.callers_context;
    // if (pump_monitor == NULL)
    // {
    //     ESP_LOGE(TAG, "Pump monitor context is NULL in overcurrent callback");
    //     return;
    // }
    // for (int i = 0; i < PUMP_MONITOR_MAXIMUM_SUBSCRIBER; ++i)
    // {
    //     if (pump_monitor->subscribers[i].in_use)
    //     {
    //         pump_monitor_event_hook_t *hook = &pump_monitor->subscribers[i].hook;
    //         if (hook && hook->callback) 
    //         {
    //             hook->callback(hook->context,
    //                            hook->actuator_id,
    //                            event,
    //                            pump_monitor->config->id);
    //         }
    //         else
    //         {
    //             ESP_LOGW(TAG, "Skipping subscriber %d - no callback assigned", pump_monitor->subscribers[i].id);
    //         }
    //     }
    // }
}

// static void pump_monitor_continuous_read_callback(measurement_item_t item) {
//     if(item.callers_context == NULL) {
//         ESP_LOGE(TAG, "Pump monitor context is NULL in overcurrent callback");
//         return;
//     }
//     pump_monitor_t *pump_monitor = (pump_monitor_t *)item.callers_context;
//     // take mutex from isr
//     if (xSemaphoreTakeFromISR(pump_monitor->read_mutex, NULL) == pdTRUE) {
//         pump_monitor->value_ready = true;
//         pump_monitor->measurement_item = item;
//         xSemaphoreGiveFromISR(pump_monitor->read_mutex, NULL);
//     } else {
//         ESP_LOGE(TAG, "Failed to take read mutex in ISR");
//     }
//     ESP_LOGI(TAG, "read value ready in pump monitor callback");
// }

error_type_t pump_monitor_subscribe_event(pump_monitor_t *pump_monitor, const pump_monitor_subscriber_t* subscriber, int *event_id)
{
    if (pump_monitor == NULL || subscriber == NULL || event_id == NULL)
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
    ESP_LOGI(TAG,"setting subscriber callback in slot %d for subscriber id %d with context %p and callback %p", slot, subscriber->id, subscriber->context, subscriber->callback);
    pump_monitor->subscribers[slot].callback = subscriber->callback;
    pump_monitor->subscribers[slot].id = slot;
    pump_monitor->subscribers[slot].in_use = true;
    pump_monitor->subscribers[slot].context = subscriber->context;
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
    pump_monitor->subscriber_count--;

    ESP_LOGI(TAG, "Unsubscribed event id=%d", event_id);

    return SYSTEM_OK;
}
