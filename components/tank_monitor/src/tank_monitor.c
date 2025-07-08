#include <tank_monitor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tank_monitor_t {
    tank_monitor_config_t *config; // Pointer to the tank monitor configuration
    tank_monitor_state_t state; // State of the tank monitor
    tank_state_machine_state_t tank_state; // State of the tank being monitored
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
    return monitor;
}

error_type_t tank_monitor_init(tank_monitor_t *monitor) {
    if (monitor == NULL || monitor->config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or configuration
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

error_type_t tank_monitor_get_state(tank_monitor_t *monitor, tank_monitor_state_t *state) {
    if (monitor == NULL || state == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or state pointer
    }

    *state = monitor->state;
    return SYSTEM_OK;
}

error_type_t tank_monitor_get_config(tank_monitor_t *monitor, tank_monitor_config_t *config) {
    if (monitor == NULL || config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or configuration pointer
    }

    memcpy(config, monitor->config, sizeof(tank_monitor_config_t));
    return SYSTEM_OK;
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
        // Call the event callback if defined
        if (monitor->config->event_callback != NULL) {
            monitor->config->event_callback(monitor->tank_state);
        }
    }

    return SYSTEM_OK;
}