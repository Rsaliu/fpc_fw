#include <tank_monitor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define  TANK_MONITOR_MAXIMUM_SUBSCRIBER 10 // Maximum number of tank subscribers

typedef struct{
    int id; // Unique identifier for the subscriber
    tank_monitor_event_hook_t* hook; // Callback function for the subscriber
}tank_monitor_subscriber_t;

struct tank_monitor_t {
    tank_monitor_config_t *config; // Pointer to the tank monitor configuration
    tank_monitor_state_t state; // State of the tank monitor
    tank_state_machine_state_t tank_state; // State of the tank being monitored
    tank_monitor_subscriber_t* subscribers[TANK_MONITOR_MAXIMUM_SUBSCRIBER]; // Callback for tank state events
    int subscriber_count; // Count of subscribers
};

//dummy level_sensor object
error_type_t level_sensor_get_level_in_mm(level_sensor_t *sensor, int *level) {
    if (sensor == NULL || level == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null sensor or level pointer
    }
    // Simulate getting the level from the sensor
    *level = 100; // Example level in mm
    printf("Level sensor ID: %d, Current level: %d mm\n", sensor->id, *level);
    return SYSTEM_OK;
}
//

tank_monitor_t* tank_monitor_create(tank_monitor_config_t config) {
    tank_monitor_t *monitor = (tank_monitor_t *)malloc(sizeof(tank_monitor_t));
    if (monitor == NULL) {
        return NULL; // Handle memory allocation failure
    }

    monitor->config = (tank_monitor_config_t *)malloc(sizeof(tank_monitor_config_t));
    if (monitor->config == NULL) {
        free(monitor);
        return NULL; // Handle memory allocation failure
    }

    memcpy(monitor->config, &config, sizeof(tank_monitor_config_t));
    monitor->state = TANK_MONITOR_NOT_INITIALIZED;
    monitor->tank_state = TANK_STATE_MACHINE_NORMAL_STATE; // Initialize tank state to normal
    monitor->subscriber_count = 0; // Initialize subscriber count to 0

    return monitor;
}

error_type_t tank_monitor_init(tank_monitor_t *monitor) {
    if (monitor == NULL || monitor->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or configuration
    }
    // Validate the tank configuration
    if (monitor->config->tank == NULL || monitor->config->sensor == NULL) {
        return SYSTEM_INVALID_PARAMETER; // Handle invalid configuration
    }
    if (monitor->state != TANK_MONITOR_NOT_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Monitor is already initialized
    }

    monitor->state = TANK_MONITOR_INITIALIZED;

    return SYSTEM_OK;
}

error_type_t tank_monitor_deinit(tank_monitor_t *monitor) {
    if (monitor == NULL || monitor->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or configuration
    }

    if (monitor->state == TANK_MONITOR_NOT_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Monitor is not initialized
    }

    monitor->state = TANK_MONITOR_NOT_INITIALIZED;
    printf("Tank monitor deinitialized\n");

    return SYSTEM_OK;
}

error_type_t tank_monitor_destroy(tank_monitor_t **monitor) {
    if (*monitor == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor
    }

    if ((*monitor)->config != NULL) {
        free((*monitor)->config);
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

    memcpy(config, monitor->config, sizeof(tank_monitor_config_t));
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
    if (monitor == NULL || monitor->config == NULL || monitor->config->sensor == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or sensor
    }

    // Simulate checking the level sensor
    int current_level;
    error_type_t err = level_sensor_get_level_in_mm(monitor->config->sensor, &current_level);
    if(err != SYSTEM_OK) {
        return err; // Handle error in getting level from sensor
    }
    tank_config_t tank_config;
    err = tank_get_config(monitor->config->tank, &tank_config); 
    if(err != SYSTEM_OK) {
        return err; // Handle error in getting tank configuration
    }
    int full_level = tank_config.full_level_in_mm;
    int low_level = tank_config.low_level_in_mm;

    //state machine logic
    tank_state_machine_state_t previous_state = monitor->tank_state;
    switch (monitor->tank_state) {
        case TANK_STATE_MACHINE_NORMAL_STATE:
            if (current_level >= full_level) {
                monitor->tank_state = TANK_STATE_MACHINE_FULL_STATE;
                printf("Tank is full\n");
            } else if (current_level < low_level) {
                monitor->tank_state = TANK_STATE_MACHINE_LOW_STATE;
                printf("Tank is below low level\n");
            }
            break;

        case TANK_STATE_MACHINE_FULL_STATE:
            if (current_level < full_level) {
                monitor->tank_state = TANK_STATE_MACHINE_NORMAL_STATE;
                printf("Tank is not full anymore\n");
            }
            break;

        case TANK_STATE_MACHINE_LOW_STATE:
            if (current_level >= low_level) {
                monitor->tank_state = TANK_STATE_MACHINE_NORMAL_STATE;
                printf("Tank is back to normal state\n");
            }
            break;

        default:
            return SYSTEM_INVALID_STATE; // Invalid state
    }
    if(monitor->tank_state != previous_state) {
        // Notify subscribers about the state change
        for (int i = 0; i < monitor->subscriber_count; i++) {
            if (monitor->subscribers[i] != NULL && monitor->subscribers[i]->hook != NULL && monitor->subscribers[i]->hook->callback != NULL) {
                monitor->subscribers[i]->hook->callback(
                    monitor->subscribers[i]->hook->context,
                    monitor->subscribers[i]->hook->actuator_id,
                    state_machine_state_to_event(monitor->tank_state),
                    monitor->config->id
                ); // Call the subscriber's callback
            }
        }
    }

    return SYSTEM_OK;
}

error_type_t tank_monitor_subscribe_event(tank_monitor_t *monitor, const tank_monitor_event_hook_t* hook,int* event_id)
{
    if (monitor == NULL || hook == NULL || event_id == NULL) {
        printf("Error: Null parameter in tank_monitor_subscribe_event\n");
        return SYSTEM_NULL_PARAMETER; // Handle null monitor, callback, or event_id pointer
    }
    printf("monitor pointer: %p, hook pointer: %p, event_id pointer: %p\n", monitor, hook, event_id);
    // Check if the monitor is initialized
    if (monitor->state != TANK_MONITOR_INITIALIZED) {
        printf("Tank monitor is not initialized\n");
        return SYSTEM_INVALID_STATE; // Monitor is not initialized
    }
    // Check if the maximum number of subscribers is reached
    if (monitor->subscriber_count >= TANK_MONITOR_MAXIMUM_SUBSCRIBER) {
        return SYSTEM_BUFFER_OVERFLOW; // Maximum number of subscribers reached
    }

    tank_monitor_subscriber_t *subscriber = (tank_monitor_subscriber_t *)malloc(sizeof(tank_monitor_subscriber_t));
    if (subscriber == NULL) {
        return SYSTEM_FAILED; // Handle memory allocation failure
    }
    subscriber->id = monitor->subscriber_count; // Assign a unique ID to the subscriber
    subscriber->hook = (tank_monitor_event_hook_t *)malloc(sizeof(tank_monitor_event_hook_t));
    if (subscriber->hook == NULL) {
        free(subscriber); // Free the subscriber if hook allocation fails
        return SYSTEM_FAILED; // Handle memory allocation failure
    }
    // copy using memcpy to avoid issues with pointer assignment
    memcpy(subscriber->hook, hook, sizeof(tank_monitor_event_hook_t));
    monitor->subscribers[monitor->subscriber_count++] = subscriber; // Add the subscriber to the list
    *event_id = subscriber->id; // Return the ID of the newly subscribed event


    return SYSTEM_OK;
}
error_type_t tank_monitor_unsubscribe_event(tank_monitor_t *monitor,int event_id){
    if (monitor == NULL || event_id < 0 || event_id >= monitor->subscriber_count) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or invalid event_id
    }

    // Check if the monitor is initialized
    if (monitor->state != TANK_MONITOR_INITIALIZED) {
        return SYSTEM_INVALID_STATE; // Monitor is not initialized
    }

    // Free the subscriber and remove it from the list
    free(monitor->subscribers[event_id]->hook); // Free the hook
    free(monitor->subscribers[event_id]);
    monitor->subscribers[event_id] = NULL;

    // Shift remaining subscribers down
    for (int i = event_id; i < monitor->subscriber_count - 1; i++) {
        monitor->subscribers[i] = monitor->subscribers[i + 1];
    }
    monitor->subscriber_count--; // Decrease the count of subscribers

    return SYSTEM_OK;
}

error_type_t tank_monitor_print_info(tank_monitor_t* monitor){
    printf("Tank ID: %d\n", monitor->config->id);
    printf("Subscriber Count: %d\n",monitor->subscriber_count );
    printf("State: %d\n", monitor->state);
    return SYSTEM_OK;
}

error_type_t tank_monitor_print_info_into_buffer(tank_monitor_t* monitor,char* buffer, const size_t buffer_size){
    int written = snprintf(buffer,buffer_size, "Tank ID: %d\n Subsciber Count: %d\n State: %d\n",
       monitor->config->id, monitor->subscriber_count,monitor->state);
       if (written < 0)return SYSTEM_OPERATION_FAILED;
       
       if ((size_t)written > buffer_size)
       {
            return SYSTEM_BUFFER_OVERFLOW;
       }

       return SYSTEM_OK;
       
}