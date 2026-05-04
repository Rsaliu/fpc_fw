#include <tank_monitor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

#define  TANK_MONITOR_MAXIMUM_SUBSCRIBER 10 // Maximum number of tank subscribers
#define TANK_MONITOR_MAX_SAMPLE_SIZE 10 // Maximum number of samples for averaging level readings, can be adjusted based on requirements

static const char*TAG = "TANK_MONITOR";

struct tank_monitor_t {
    tank_monitor_config_t config; // Pointer to the tank monitor configuration
    uint16_t sampled_level_values[TANK_MONITOR_MAX_SAMPLE_SIZE]; // Buffer to hold sampled level values for averaging (if applicable)
    tank_monitor_state_t state; // State of the tank monitor
    tank_state_machine_state_t state_machine_state; // State of the tank being monitored
    tank_monitor_subscriber_t subscribers[TANK_MONITOR_MAXIMUM_SUBSCRIBER]; // Callback for tank state events
    int subscriber_count; // Count of subscribers
};

//dummy level_sensor object
// error_type_t 
// level_sensor_get_level_in_mm(level_sensor_t *sensor, int *level) {
//     ESP_LOGI(TAG, "Checking sensor pointer: %p, level pointer: %p", sensor, level);
//     if (sensor == NULL || level == NULL) {
//         return SYSTEM_NULL_PARAMETER; // Handle null sensor or level pointer
//     }
//     // Simulate getting the level from the sensor
//     *level = 100; // Example level in mm
//     ESP_LOGI(TAG,"Level sensor ID: %d, Current level: %d mm\n", sensor->id, *level);
//     return SYSTEM_OK;
// }
//

tank_monitor_t* tank_monitor_create(tank_monitor_config_t config) {
    tank_monitor_t *monitor = (tank_monitor_t *)malloc(sizeof(tank_monitor_t));
    if (monitor == NULL) {
        return NULL; // Handle memory allocation failure
    }
    monitor->config = config;
    monitor->state = TANK_MONITOR_NOT_INITIALIZED;
    monitor->state_machine_state = TANK_STATE_MACHINE_NORMAL_STATE; // Initialize tank state to normal
    monitor->subscriber_count = 0; // Initialize subscriber count to 0
    for(int i = 0; i < TANK_MONITOR_MAXIMUM_SUBSCRIBER; ++i) {
        monitor->subscribers[i].id = -1; // Initialize subscriber slots to NULL
        monitor->subscribers[i].in_use = false; // Mark subscriber slots as not in use
    }
     ESP_LOGI(TAG, "Tank monitor created with ID: %d\n", config.id);
    return monitor;
}

error_type_t tank_monitor_init(tank_monitor_t *monitor) {
    if (monitor == NULL || monitor->config.tank == NULL || monitor->config.sensor == NULL || monitor->config.level_read_cb == NULL || monitor->config.analytics_cb == NULL) {
        ESP_LOGE(TAG, "Null parameter in tank monitor configuration: monitor=%p, tank=%p, sensor=%p, level_read_cb=%p, analytics_cb=%p", 
                 monitor, monitor ? monitor->config.tank : NULL, monitor ? monitor->config.sensor : NULL, monitor ? monitor->config.level_read_cb : NULL, monitor ? monitor->config.analytics_cb : NULL);
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or configuration
    }
    if(monitor->config.number_of_samples_for_average <= 0 || monitor->config.number_of_samples_for_average > TANK_MONITOR_MAX_SAMPLE_SIZE) {
        ESP_LOGE(TAG, "Invalid number of samples for average: %d", monitor->config.number_of_samples_for_average);
        return SYSTEM_INVALID_PARAMETER; // Handle invalid number of samples for averaging
    }
    if (monitor->state != TANK_MONITOR_NOT_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Monitor is already initialized
    }

    monitor->state = TANK_MONITOR_INITIALIZED;

    return SYSTEM_OK;
}

error_type_t tank_monitor_deinit(tank_monitor_t *monitor) {
    if (monitor == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or configuration
    }

    if (monitor->state == TANK_MONITOR_NOT_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Monitor is not initialized
    }

    monitor->state = TANK_MONITOR_NOT_INITIALIZED;
    ESP_LOGI(TAG,"Tank monitor deinitialized\n");

    return SYSTEM_OK;
}

error_type_t tank_monitor_destroy(tank_monitor_t **monitor) {
    if (*monitor == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor
    }

    if((*monitor)->state != TANK_MONITOR_INITIALIZED) {
        tank_monitor_deinit(*monitor); // Deinitialize if not already deinitialized
    }
    
    free(*monitor);
    *monitor = NULL; // Set pointer to NULL after freeing

    return SYSTEM_OK;
}

error_type_t tank_monitor_get_state(const tank_monitor_t *monitor, tank_monitor_state_t *state) {
    if (monitor == NULL || state == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or state pointer
    }

    *state = monitor->state;
    return SYSTEM_OK;
}

error_type_t tank_monitor_get_config(const tank_monitor_t *monitor, tank_monitor_config_t *config) {
    if (monitor == NULL || config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or configuration pointer
    }

    memcpy(config, &monitor->config, sizeof(tank_monitor_config_t));
    return SYSTEM_OK;
}
static event_type_t state_machine_state_to_event(tank_state_machine_state_t state) {
    switch (state) {
        case TANK_STATE_MACHINE_NORMAL_STATE:
            return EVENT_TANK_NORMAL_STATE; // Normal state of the tank
        case TANK_STATE_MACHINE_FULL_STATE:
            return EVENT_TANK_FULL_STATE; // Tank is full
        case TANK_STATE_MACHINE_LOW_STATE:
            return EVENT_TANK_LOW_STATE; // Tank is below low level
        default:
            return EVENT_UNKNOWN; // Unknown state
    }
}
error_type_t tank_monitor_check_level(tank_monitor_t *monitor) {
    if (monitor == NULL || monitor->config.sensor == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or sensor
    }
    level_analytics_callback_t analytics_cb = monitor->config.analytics_cb;
    if(analytics_cb == NULL) {
        ESP_LOGE(TAG, "Analytics callback is NULL in tank monitor configuration");
        return SYSTEM_NULL_PARAMETER; // Handle null analytics callback
    }
     ESP_LOGI(TAG, "Checking tank level for monitor ID: %d\n", monitor->config.id);
     ESP_LOGI(TAG, "Tank pointer: %p, Sensor pointer: %p\n", monitor->config.tank, monitor->config.sensor);
    for(int i = 0; i < monitor->config.number_of_samples_for_average; ++i) {
        error_type_t err = monitor->config.level_read_cb(monitor->config.sensor, &monitor->sampled_level_values[i]);
        if (err != SYSTEM_OK) {
            ESP_LOGE(TAG, "Failed to read level from sensor for sample %d: %d", i, err);
            return err; // Handle error in reading level from sensor
        }
            ESP_LOGI(TAG, "Sampled level value for sample %d: %d mm\n", i, monitor->sampled_level_values[i]);
    }
     ESP_LOGI(TAG, "Sampled level values:");
    error_type_t err;
    tank_config_t tank_config;
    err = tank_get_config(monitor->config.tank, &tank_config); 
    if(err != SYSTEM_OK) {
        ESP_LOGE(TAG, "failed to get tank");
        return err; // Handle error in getting tank configuration
    }
    int full_level = tank_config.full_level_in_mm;
    int low_level = tank_config.low_level_in_mm;

    //state machine logic
    tank_state_machine_state_t previous_state = monitor->state_machine_state;
    if(!monitor->config.analytics_cb) {
        ESP_LOGE(TAG, "Analytics callback is NULL in tank monitor configuration");
        return SYSTEM_NULL_PARAMETER; // Handle null analytics callback
    }
    err = analytics_cb(monitor->sampled_level_values, monitor->config.number_of_samples_for_average, full_level, low_level, &monitor->state_machine_state);
    if(err != SYSTEM_OK) {
        ESP_LOGE(TAG, "Error in analytics callback: %d", err);
        return err; // Handle error in analytics callback
    }

     ESP_LOGI(TAG, "Tank state machine state: %d\n", monitor->state_machine_state);
    if(monitor->state_machine_state != previous_state) {
        // Notify subscribers about the state change
        for (int i = 0; i < monitor->subscriber_count; i++) {
                if(monitor->subscribers[i].context == NULL){
                    ESP_LOGE(TAG, "Subscriber context is NULL for subscriber %d", monitor->subscribers[i].id);
                    continue;
                }

                if (monitor->subscribers[i].callback) 
                {
                    monitor->subscribers[i].callback((monitor->subscribers[i].context),
                                   state_machine_state_to_event(monitor->state_machine_state),
                                   monitor->subscribers[i].id);
                }
                else
                {
                    ESP_LOGW(TAG, "Skipping subscriber %d - no callback assigned", monitor->subscribers[i].id);
                }
        }
    }

    return SYSTEM_OK;
}

error_type_t tank_monitor_subscribe_event(tank_monitor_t *monitor, const tank_monitor_subscriber_t* subscriber,int* event_id)
{
    if (monitor == NULL || subscriber == NULL || event_id == NULL)
    {
        ESP_LOGE(TAG, "Null parameter in monitor_subscribe_event");
        return SYSTEM_NULL_PARAMETER;
    }

    if (monitor->state != TANK_MONITOR_INITIALIZED)
    {
        ESP_LOGE(TAG, "Pump monitor is not initialized");
        return SYSTEM_INVALID_STATE;
    }

    if (monitor->subscriber_count >= TANK_MONITOR_MAXIMUM_SUBSCRIBER)
    {
        return SYSTEM_BUFFER_OVERFLOW;
    }

    /* find first free slot */
    int slot = -1;
    for (int i = 0; i < TANK_MONITOR_MAXIMUM_SUBSCRIBER; ++i)
    {
        if (!monitor->subscribers[i].in_use) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        return SYSTEM_BUFFER_OVERFLOW;
    }

    monitor->subscribers[slot].callback = subscriber->callback;
    monitor->subscribers[slot].id = slot;
    monitor->subscribers[slot].in_use = true;
    monitor->subscribers[slot].context = subscriber->context;
    monitor->subscriber_count++;

    *event_id = monitor->subscribers[slot].id;

    ESP_LOGI(TAG, "Subscribed event id=%d (slot=%d)", *event_id, slot);
    return SYSTEM_OK;
}

error_type_t tank_monitor_unsubscribe_event(tank_monitor_t *monitor,int event_id){
    if (monitor == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }

    if (event_id < 0 || event_id >= TANK_MONITOR_MAXIMUM_SUBSCRIBER) {
        return SYSTEM_INVALID_PARAMETER;
    }

    if (monitor->state != TANK_MONITOR_INITIALIZED)
    {
        return SYSTEM_INVALID_STATE;
    }

    if (!monitor->subscribers[event_id].in_use) {
        return SYSTEM_INVALID_PARAMETER; 
    }

    /* clear the slot */
    monitor->subscribers[event_id].in_use = false;
    monitor->subscribers[event_id].id = -1;
    monitor->subscriber_count--;

    ESP_LOGI(TAG, "Unsubscribed event id=%d", event_id);

    return SYSTEM_OK;
}

error_type_t tank_monitor_print_info(tank_monitor_t* monitor){
    ESP_LOGI(TAG,"Tank ID: %d\n", monitor->config.id);
    ESP_LOGI(TAG,"Subscriber Count: %d\n",monitor->subscriber_count );
    ESP_LOGI(TAG,"State: %d\n", monitor->state);
    return SYSTEM_OK;
}

error_type_t tank_monitor_print_info_into_buffer(tank_monitor_t* monitor,char* buffer, const size_t buffer_size){
    int written = snprintf(buffer,buffer_size, "Tank ID: %d\n Subsciber Count: %d\n State: %d\n",
       monitor->config.id, monitor->subscriber_count,monitor->state);
       if (written < 0)return SYSTEM_OPERATION_FAILED;
       
       if ((size_t)written > buffer_size)
       {
            return SYSTEM_BUFFER_OVERFLOW;
       }
       return SYSTEM_OK;
}