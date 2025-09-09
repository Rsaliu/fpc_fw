#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "pump_monitor.h"

static const char *TAG = "pump_monitor";

#define PUMP_MONITOR_MAXIMUM_SUBSCRIBER 10 // Maximum number of pump subscribers

typedef struct
{
    int id;                          // Unique identifier for the subscriber
    pump_monitor_event_hook_t *hook; // Callback function for the subscriber
} pump_monitor_subscriber_t;

struct pump_monitor_t
{
    pump_monitor_config_t *config;                                           // Pointer to the pump monitor configuration
    pump_monitor_state_t state;                                              // State of the pump monitor
    pump_state_machine_state_t pump_state;                                   // State of the pump being monitored
    pump_monitor_subscriber_t *subscribers[PUMP_MONITOR_MAXIMUM_SUBSCRIBER]; // Callback for pump state events
    int subscriber_count;                                                    // Count of subscribers
};



pump_monitor_t *pump_monitor_create(pump_monitor_config_t config)
{
    pump_monitor_t *pump_monitor = (pump_monitor_t *)malloc(sizeof(pump_monitor_t));
    if (pump_monitor == NULL)
    {
        return NULL; // Handle memory allocation failure
    }

    pump_monitor->config = (pump_monitor_config_t *)malloc(sizeof(pump_monitor_config_t));
    if (pump_monitor->config == NULL)
    {
        free(pump_monitor);
        return NULL; // Handle memory allocation failure
    }

    memcpy(pump_monitor->config, &config, sizeof(pump_monitor_config_t));
    pump_monitor->state = PUMP_MONITOR_NOT_INITIALIZED;
    pump_monitor->pump_state = PUMP_STATE_MACHINE_NORMAL_STATE; // Initialize pump state to normal
    pump_monitor->subscriber_count = 0;                         // Initialize subscriber count to 0

    return pump_monitor;
}

error_type_t pump_monitor_init(pump_monitor_t *pump_monitor)
{
    if (pump_monitor == NULL || pump_monitor->config == NULL)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null pump_monitor or configuration
    }
    // Validate the pump configuration
    if (pump_monitor->config->pump == NULL || pump_monitor->config->sensor == NULL)
    {
        return SYSTEM_INVALID_PARAMETER; // Handle invalid configuration
    }
    if (pump_monitor->state != PUMP_MONITOR_NOT_INITIALIZED)
    {
        return SYSTEM_INVALID_STATE; // pump_monitor is already initialized
    }

    pump_monitor->state = PUMP_MONITOR_INITIALIZED;
    ESP_LOGI(TAG, "Pump monitor initialized");
    return SYSTEM_OK;
}

error_type_t pump_monitor_deinit(pump_monitor_t *pump_monitor)
{
    if (pump_monitor == NULL || pump_monitor->config == NULL)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null pump_monitor or configuration
    }

    if (pump_monitor->state == PUMP_MONITOR_NOT_INITIALIZED)
    {
        return SYSTEM_INVALID_STATE; // pump_monitor is not initialized
    }

    pump_monitor->state = PUMP_MONITOR_NOT_INITIALIZED;
    ESP_LOGI(TAG, "Pump monitor deinitialized");
    return SYSTEM_OK;
}

error_type_t pump_monitor_destroy(pump_monitor_t **pump_monitor)
{
    if (*pump_monitor == NULL)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null pump_monitor
    }

    if ((*pump_monitor)->config != NULL)
    {
        free((*pump_monitor)->config);
    }

    free(*pump_monitor);
    *pump_monitor = NULL; // Set pointer to NULL after freeing

    ESP_LOGI(TAG, "Pump monitor destroyed");
    return SYSTEM_OK;
}

error_type_t pump_monitor_get_state(const pump_monitor_t *pump_monitor, pump_monitor_state_t *state)
{
    if (pump_monitor == NULL || state == NULL)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null pump_monitor or state pointer
    }

    *state = pump_monitor->state;
    return SYSTEM_OK;
}

error_type_t pump_monitor_get_config(const pump_monitor_t *pump_monitor, pump_monitor_config_t *config)
{
    if (pump_monitor == NULL || config == NULL)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null pump_monitor or configuration pointer
    }

    memcpy(config, pump_monitor->config, sizeof(pump_monitor_config_t));
    return SYSTEM_OK;
}

static event_type_t state_machine_state_to_event(pump_state_machine_state_t state)
{
    switch (state)
    {
    case PUMP_STATE_MACHINE_NORMAL_STATE:
        return EVENT_PUMP_NORMAL; // Normal current supply state of the pump

    case PUMP_STATE_MACHINE_UNDERCURRENT_STATE:
        return EVENT_PUMP_UNDERCURRENT; // pump current supply is less than the current rating

    case PUMP_STATE_MACHINE_OVERCURRENT_STATE:
        return EVENT_PUMP_OVERCURRENT; // pump current supply is greater than the current rating

    default:
        return EVENT_UNKNOWN; // Unknown state
    }
}

error_type_t pump_monitor_check_current(pump_monitor_t *pump_monitor)
{
    if (pump_monitor == NULL || pump_monitor->config == NULL || pump_monitor->config->sensor == NULL)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null pump_monitor or sensor
    }

    // Simulate checking the current sensor
    float current_amp_reading;
    error_type_t err = current_sensor_get_current_in_amp(pump_monitor->config->sensor, &current_amp_reading);

    if (err != SYSTEM_OK)
    {
        return err; // Handle error in getting level from sensor
    }
    pump_config_t pump_config;
    err = pump_get_config(pump_monitor->config->pump, &pump_config);
    if (err != SYSTEM_OK)
    {
        return err; // Handle error in getting tank configuration
    }

    float normal_current_supply = pump_config.current_rating;
    float low_current_supply = normal_current_supply - normal_current_supply * 0.05;
    float high_current_supply = normal_current_supply + normal_current_supply * 0.05;

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
        return SYSTEM_INVALID_STATE; // Invalid state
    }
    if (pump_monitor->pump_state != previous_state)
    {
        // Notify subscribers about the state change
        for (int i = 0; i < pump_monitor->subscriber_count; i++)
        {
            if (pump_monitor->subscribers[i] != NULL && pump_monitor->subscribers[i]->hook != NULL && pump_monitor->subscribers[i]->hook->callback != NULL)
            {
                pump_monitor->subscribers[i]->hook->callback(
                    pump_monitor->subscribers[i]->hook->context,
                    pump_monitor->subscribers[i]->hook->actuator_id,
                    state_machine_state_to_event(pump_monitor->pump_state),
                    pump_monitor->config->id); // Call the subscriber's callback
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
        return SYSTEM_NULL_PARAMETER; // Handle null pump_monitor, callback, or event_id pointer
    }
    ESP_LOGE(TAG, "pump_monitor pointer: %p, hook pointer: %p, event_id pointer: %p\n", pump_monitor, hook, event_id);

    //  Check if the pump_monitor is initialized
    if (pump_monitor->state != PUMP_MONITOR_INITIALIZED)
    {
        // printf("Pump monitor is not initialized\n");
        ESP_LOGE(TAG, "Pump monitor is not initialized");
        return SYSTEM_INVALID_STATE; // pump_monitor is not initialized
    }
    // Check if the maximum number of subscribers is reached
    if (pump_monitor->subscriber_count >= PUMP_MONITOR_MAXIMUM_SUBSCRIBER)
    {
        return SYSTEM_BUFFER_OVERFLOW; // Maximum number of subscribers reached
    }

    pump_monitor_subscriber_t *subscriber = (pump_monitor_subscriber_t *)malloc(sizeof(pump_monitor_subscriber_t));
    if (subscriber == NULL)
    {
        return SYSTEM_FAILED; // Handle memory allocation failure
    }
    subscriber->id = pump_monitor->subscriber_count; // Assign a unique ID to the subscriber
    subscriber->hook = (pump_monitor_event_hook_t *)malloc(sizeof(pump_monitor_event_hook_t));
    if (subscriber->hook == NULL)
    {
        free(subscriber);     // Free the subscriber if hook allocation fails
        return SYSTEM_FAILED; // Handle memory allocation failure
    }
    // copy using memcpy to avoid issues with pointer assignment
    memcpy(subscriber->hook, hook, sizeof(pump_monitor_event_hook_t));
    pump_monitor->subscribers[pump_monitor->subscriber_count++] = subscriber; // Add the subscriber to the list
    *event_id = subscriber->id;                                               // Return the ID of the newly subscribed event

    return SYSTEM_OK;
}
error_type_t pump_monitor_unsubscribe_event(pump_monitor_t *pump_monitor, int event_id)
{
    if (pump_monitor == NULL || event_id < 0 || event_id >= pump_monitor->subscriber_count)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null pump_monitor or invalid event_id
    }

    // Check if the pump_monitor is initialized
    if (pump_monitor->state != PUMP_MONITOR_INITIALIZED)
    {
        return SYSTEM_INVALID_STATE; // pump_monitor is not initialized
    }

    // Free the subscriber and remove it from the list
    free(pump_monitor->subscribers[event_id]->hook); // Free the hook
    free(pump_monitor->subscribers[event_id]);
    pump_monitor->subscribers[event_id] = NULL;

    // Shift remaining subscribers down
    for (int i = event_id; i < pump_monitor->subscriber_count - 1; i++)
    {
        pump_monitor->subscribers[i] = pump_monitor->subscribers[i + 1];
    }
    pump_monitor->subscriber_count--; // Decrease the count of subscribers

    return SYSTEM_OK;
}