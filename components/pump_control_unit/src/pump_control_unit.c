#include <pump_control_unit.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "esp_log.h"
#define  PUMP_CONTROL_UNIT_MAX_TANK_MONITORS  10 // Maximum number of tank monitors
#define PUMP_CONTROL_UNIT_MAX_PUMP_MONITORS PUMP_CONTROL_UNIT_MAX_TANK_MONITORS // For compatibility with existing code
#define PUMP_CONTROL_UNIT_MAX_RELAY_COUNT PUMP_CONTROL_UNIT_MAX_PUMP_MONITORS // Maximum number of relays
#define PUMP_CONTROL_UNIT_MAX_EVENT_MAP PUMP_CONTROL_UNIT_MAX_PUMP_MONITORS // Maximum number of event maps
#define LOAD_FACTOR 0.75 // Load factor for the hash map


typedef struct{
    int monitor_id; // Unique identifier for the monitor
    int relay_id; // Unique identifier for the relay
}monitor_relay_group_t;

//dummy pump_monitorGetConfig function
error_type_t pump_monitorGetConfig(pump_monitor_t *monitor, pump_monitor_config_t *config) {
    if (monitor == NULL || config == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null monitor or configuration pointer
    }
    config->id = 1; // Assuming pump_monitor_t has an id field
    config->pump = monitor->pump; // Assuming pump_monitor_t has a pump field
    config->current_sensor = monitor->current_sensor; // Assuming pump_monitor_t has a current_sensor field
    return SYSTEM_OK;
}


struct pump_control_unit_t {
    HashMap pump_monitors_map; // HashMap to store pump monitors
    HashMap tank_monitors_map; // HashMap to store tank monitors
    HashMap relays_map; // HashMap to store relays
    HashMap pump_monitor_event_map; // HashMap to store pump monitor events
    HashMap tank_monitor_event_map; // HashMap to store tank monitor events
    bool is_initialized; // Flag to check if the manager is initialized
};

const char *PUMP_CONTROL_UNIT_TAG = "PUMP_CONTROL_UNIT";

static monitor_relay_group_t * get_group_from_tank_monitor_event_map(HashMap* map, int tank_monitor_id, int relay_id,int *event_id) {
    if (map == NULL || event_id == NULL) {
        return NULL; // Handle null map
    }
    if (tank_monitor_id < 0 || relay_id < 0) {
        return NULL; // Handle invalid IDs
    }
    ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Searching for group with tank monitor ID: %d and relay ID: %d", tank_monitor_id, relay_id);
    // Iterate through the map to find the event ID associated with the tank monitor ID and relay ID
    MapIterator it = emhashmap_iterator(map);
    MapEntry* entry = emhashmap_iterator_next(&it);
    while (entry != NULL) {
        monitor_relay_group_t *group = (monitor_relay_group_t *)entry->value;
        if (group->monitor_id == tank_monitor_id && group->relay_id == relay_id) {
            *event_id = entry->key; // Set the event ID
            ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Found group for tank monitor ID: %d, relay ID: %d, event ID: %d", tank_monitor_id, relay_id, *event_id);
            return entry->value; // Return the group if found
        }
        entry = emhashmap_iterator_next(&it); // Move to the next entry
    }

    return NULL; // Return NULL if not found
}

pump_control_unit_t* pump_control_unit_create() {
    pump_control_unit_t *manager = (pump_control_unit_t *)malloc(sizeof(pump_control_unit_t));
    if (manager == NULL) {
        return NULL; // Handle memory allocation failure
    }
    emhashmap_initialize(&manager->pump_monitors_map,PUMP_CONTROL_UNIT_MAX_TANK_MONITORS,LOAD_FACTOR); // Initialize pump monitors map
    emhashmap_initialize(&manager->tank_monitors_map,PUMP_CONTROL_UNIT_MAX_TANK_MONITORS,LOAD_FACTOR); // Initialize tank monitors map
    emhashmap_initialize(&manager->relays_map,PUMP_CONTROL_UNIT_MAX_RELAY_COUNT,LOAD_FACTOR); // Initialize relays map
    emhashmap_initialize(&manager->pump_monitor_event_map,PUMP_CONTROL_UNIT_MAX_EVENT_MAP,LOAD_FACTOR); // Initialize pump monitor events map
    emhashmap_initialize(&manager->tank_monitor_event_map,PUMP_CONTROL_UNIT_MAX_EVENT_MAP,LOAD_FACTOR); // Initialize tank monitor events map

    manager->is_initialized = false;

    return manager;
}

error_type_t pump_control_unit_init(pump_control_unit_t *manager) {
    if (manager == NULL ) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager or configuration
    }
    if (manager->is_initialized) {
        return SYSTEM_INVALID_STATE; // Manager is already initialized
    }
    manager->is_initialized = true;
    return SYSTEM_OK;
}

error_type_t pump_control_unit_get_state(const pump_control_unit_t *manager, bool *is_initialized) {
    if (manager == NULL || is_initialized == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager or state pointer
    }

    *is_initialized = manager->is_initialized;
    return SYSTEM_OK;
}


error_type_t pump_control_unit_deinit(pump_control_unit_t *manager) {
    if (manager == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager
    }

    if (!manager->is_initialized) {
        return SYSTEM_INVALID_STATE; // Manager is not initialized
    }

    manager->is_initialized = false;
    return SYSTEM_OK;
}

error_type_t pump_control_unit_destroy(pump_control_unit_t **manager) {
    if (*manager == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager
    }

    emhashmap_deinitialize(&(*manager)->pump_monitors_map); // Deinitialize pump monitors map
    emhashmap_deinitialize(&(*manager)->tank_monitors_map); // Deinitialize tank monitors map
    emhashmap_deinitialize(&(*manager)->relays_map); // Deinitialize relays map
    free(*manager);
    *manager = NULL; // Set to NULL after freeing
    return SYSTEM_OK;
}

error_type_t pump_control_unit_add_tank_monitor(pump_control_unit_t *manager, tank_monitor_t *tank_monitor) {
    if (manager == NULL || tank_monitor == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager or tank monitor
    }
    if(!manager->is_initialized) {
        return SYSTEM_INVALID_STATE; // Manager is not initialized
    }
    // Add the tank monitor to the map
    tank_monitor_config_t config;
    error_type_t err = tank_monitor_get_config(tank_monitor, &config);
    if (err != SYSTEM_OK) {
        return err; // Handle error in getting tank monitor configuration
    }
    if(emhashmap_contains(&manager->tank_monitors_map, config.id)) {
        return SYSTEM_INVALID_PARAMETER; // Tank monitor with this ID already exists
    }
    ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Adding tank monitor with ID: %d to the pump control unit", config.id);
    ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Tank monitor pointer value: %p", tank_monitor);
    if (!emhashmap_put(&manager->tank_monitors_map, config.id, (void*)tank_monitor)){
        return SYSTEM_BUFFER_OVERFLOW; // Handle error in adding to the map
    }
    
    return SYSTEM_OK;

}

error_type_t pump_control_unit_remove_tank_monitor(pump_control_unit_t *manager, int tank_monitor_id){
    if (manager == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager
    }
    if(!manager->is_initialized) {
        return SYSTEM_INVALID_STATE; // Manager is not initialized
    }
    // remove the tank monitor from the map
    if(emhashmap_remove(&manager->tank_monitors_map, tank_monitor_id) != NULL) {
        return SYSTEM_OK;
    }
    return SYSTEM_INVALID_PARAMETER; // Tank monitor not found
}

error_type_t pump_control_unit_add_pump_monitor(pump_control_unit_t *manager, pump_monitor_t* pump_monitor)
{
    if (manager == NULL || pump_monitor == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager or pump monitor
    }
    if(!manager->is_initialized) {
        return SYSTEM_INVALID_STATE; // Manager is not initialized
    }
    // Add the pump monitor to the map
    pump_monitor_config_t config;
    error_type_t err = pump_monitorGetConfig(pump_monitor, &config);
    if (err != SYSTEM_OK) {
        return err; // Handle error in getting pump monitor configuration
    }
    if(emhashmap_contains(&manager->pump_monitors_map, config.id)) {
        return SYSTEM_INVALID_PARAMETER; // Tank monitor with this ID already exists
    }
    if (!emhashmap_put(&manager->pump_monitors_map, config.id, (void*)pump_monitor)){
        return SYSTEM_BUFFER_OVERFLOW; // Handle error in adding to the map
    }
    return SYSTEM_OK;
}
error_type_t pump_control_unit_remove_pump_monitor(pump_control_unit_t *manager, int pump_monitor_id){
    if (manager == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager
    }
    if(!manager->is_initialized) {
        return SYSTEM_INVALID_STATE; // Manager is not initialized
    }
    // remove the pump monitor from the map
    if(emhashmap_remove(&manager->pump_monitors_map, pump_monitor_id) != NULL) {
        return SYSTEM_OK;
    }
    return SYSTEM_INVALID_PARAMETER; // Pump monitor not found
}

error_type_t pump_control_unit_add_relay(pump_control_unit_t *manager, relay_t *relay){
    if (manager == NULL || relay == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager or relay
    }
    if(!manager->is_initialized) {
        return SYSTEM_INVALID_STATE; // Manager is not initialized
    }
    // Add the relay to the map
    relay_config_t config;
    error_type_t err = relay_get_config(relay, &config);
    if (err != SYSTEM_OK) {
        return err; // Handle error in getting relay configuration
    }
    if(emhashmap_contains(&manager->relays_map, config.id)) {
        return SYSTEM_INVALID_PARAMETER; // Tank monitor with this ID already exists
    }
    if (!emhashmap_put(&manager->relays_map, config.relay_pin_number , (void*)relay)){
        return SYSTEM_BUFFER_OVERFLOW; // Handle error in adding to the map
    }

    return SYSTEM_OK;
}
error_type_t pump_control_unit_remove_relay(pump_control_unit_t *manager, int relay_id){
    if (manager == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager
    }
    if(!manager->is_initialized) {
        return SYSTEM_INVALID_STATE; // Manager is not initialized
    }
    // remove the relay from the map
    if(emhashmap_remove(&manager->relays_map, relay_id) != NULL) {
        return SYSTEM_OK;
    }
    return SYSTEM_INVALID_PARAMETER; // Relay not found
}

error_type_t pump_control_unit_get_relay_by_id(pump_control_unit_t *manager, int relay_id, relay_t **relay){
    if (manager == NULL || relay == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager or relay pointer
    }
    if(!manager->is_initialized) {
        return SYSTEM_INVALID_STATE; // Manager is not initialized
    }
    // Get the relay from the map
    MapEntry *entry = emhashmap_get(&manager->relays_map, relay_id);
    if (entry != NULL) {
        *relay = (relay_t *)entry->value; // Cast the value to relay_t pointer
        return SYSTEM_OK; // Successfully retrieved the relay
    }
    return SYSTEM_INVALID_PARAMETER; // Relay not found
}

error_type_t pump_control_add_relay_to_tank_monitor(pump_control_unit_t *manager, int tank_monitor_id, int relay_id)
{
    if (manager == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager or relay
    }
    if(relay_id < 0) {
        return SYSTEM_INVALID_PARAMETER; // Invalid relay ID
    }
    if(!manager->is_initialized) {
        return SYSTEM_INVALID_STATE; // Manager is not initialized
    }
    ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Adding relay ID: %d to tank monitor ID: %d after confirming init state", relay_id, tank_monitor_id);
    if(!emhashmap_contains(&manager->tank_monitors_map, tank_monitor_id)) {
        return SYSTEM_INVALID_PARAMETER; // Tank monitor not found
    }
    ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Tank monitor ID: %d found, proceeding to add relay ID: %d", tank_monitor_id, relay_id);
    tank_monitor_event_hook_t hook = {
        .context = manager, // Set the context to the pump control unit manager
        .actuator_id = relay_id, // Set the actuator ID to the relay ID
        .callback = callback_handler // Set the callback function for the event
    };
    // Subscribe to the tank monitor event
    int event_id;
    MapEntry* map_entry = emhashmap_get(&manager->tank_monitors_map, tank_monitor_id);
    if (map_entry == NULL) {
        return SYSTEM_INVALID_PARAMETER; // Tank monitor not found
    }
    tank_monitor_t *tank_monitor = (tank_monitor_t *)map_entry->value;
    error_type_t err = tank_monitor_subscribe_state_event(tank_monitor, &hook, &event_id);
    if (err != SYSTEM_OK) {
        return err; // Handle error in subscribing to the event
    }
    ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Subscribed to tank monitor ID: %d with event ID: %d", tank_monitor_id, event_id);
    
    // Create a monitor_relay_group to store the association
    monitor_relay_group_t* group = (monitor_relay_group_t*)malloc(sizeof(monitor_relay_group_t));
    if (group == NULL) {
        return SYSTEM_FAILED; // Handle memory allocation failure
    }

    group->monitor_id = tank_monitor_id; // Set the tank monitor ID
    group->relay_id = relay_id; // Set the relay ID

    // Add the event to the pump monitor event map
    if (!emhashmap_put(&manager->tank_monitor_event_map, event_id, (void*)group)) {
        return SYSTEM_BUFFER_OVERFLOW; // Handle error in adding to the map
    }
    ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Added relay ID: %d to tank monitor ID: %d with event ID: %d", relay_id, tank_monitor_id, event_id);

    return SYSTEM_OK;
}
error_type_t pump_control_unit_remove_relay_from_tank_monitor(pump_control_unit_t *manager, int tank_monitor_id, int relay_id){
    if (manager == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null manager or relay
    }
    if(relay_id < 0) {
        return SYSTEM_INVALID_PARAMETER; // Invalid relay ID
    }
    if(!manager->is_initialized) {
        return SYSTEM_INVALID_STATE; // Manager is not initialized
    }
    ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Removing relay ID: %d from tank monitor ID: %d after confirming init state", relay_id, tank_monitor_id);
    if(!emhashmap_contains(&manager->tank_monitors_map, tank_monitor_id)) {
        return SYSTEM_INVALID_PARAMETER; // Tank monitor not found
    }
    ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Tank monitor ID: %d found, proceeding to remove relay ID: %d", tank_monitor_id, relay_id);
    monitor_relay_group_t * grp;
    int event_id = -1; // Initialize event_id to -1
    grp = get_group_from_tank_monitor_event_map(&manager->tank_monitor_event_map, tank_monitor_id, relay_id,&event_id);
    if (grp == NULL || event_id < 0) {
        ESP_LOGE(PUMP_CONTROL_UNIT_TAG,"Failed to find group for tank monitor ID: %d, relay ID: %d", tank_monitor_id, relay_id);
        return SYSTEM_INVALID_PARAMETER; // Event not found for the given tank monitor and relay ID
    }
    MapEntry* map_entry = emhashmap_get(&manager->tank_monitors_map, tank_monitor_id);
    if (map_entry == NULL) {
        return SYSTEM_INVALID_PARAMETER; // Tank monitor not found
    }
    tank_monitor_t *tank_monitor = (tank_monitor_t *)map_entry->value;
    // Unsubscribe from the tank monitor event
    error_type_t err = tank_monitor_unsubscribe_state_event(tank_monitor, event_id);
    if (err != SYSTEM_OK) {
        return err; // Handle error in unsubscribing from the event
    }


    // Remove the event from the pump monitor event map
    if (emhashmap_remove(&manager->tank_monitor_event_map, event_id) == NULL) {
        return SYSTEM_INVALID_PARAMETER; // Event not found
    }
    free(grp); // Free the group memory
    return SYSTEM_OK;
}

// code commented out for now till current sensor is implemented

// error_type_t pump_control_unit_add_relay_to_pump_monitor(pump_control_unit_t *manager, int pump_monitor_id, int relay_id)
// {
//     if (manager == NULL) {
//         return SYSTEM_NULL_PARAMETER; // Handle null manager or relay
//     }
//     if(relay_id < 0) {
//         return SYSTEM_INVALID_PARAMETER; // Invalid relay ID
//     }
//     if(!manager->is_initialized) {
//         return SYSTEM_INVALID_STATE; // Manager is not initialized
//     }
//     if(!emhashmap_contains(&manager->pump_monitors_map, pump_monitor_id)) {
//         return SYSTEM_INVALID_PARAMETER; // Pump monitor not found
//     }
//     pump_monitor_event_hook_t hook = {
//         .context = manager, // Set the context to the pump control unit manager
//         .actuator_id = relay_id, // Set the actuator ID to the relay ID
//         .callback = callback_handler // Set the callback function for the event
//     };
//     // Subscribe to the pump monitor event
//     int event_id;

//     error_type_t err = pump_monitor_subscribe_event((pump_monitor_t *)emhashmap_get(&manager->pump_monitors_map, pump_monitor_id), &hook, &event_id);
//     if (err != SYSTEM_OK) {
//         return err; // Handle error in subscribing to the event
//     }
    
//     // Create a monitor_relay_group to store the association
//     monitor_relay_group_t* group = (monitor_relay_group_t*)malloc(sizeof(monitor_relay_group_t));
//     if (group == NULL) {
//         return SYSTEM_FAILED; // Handle memory allocation failure
//     }

//     // Add the event to the pump monitor event map
//     if (!emhashmap_put(&manager->pump_monitor_event_map, event_id, (void*)group)) {
//         return SYSTEM_BUFFER_OVERFLOW; // Handle error in adding to the map
//     }

//     return SYSTEM_OK;
// }
// error_type_t pump_control_unit_remove_relay_from_pump_monitor(pump_control_unit_t *manager, int pump_monitor_id, int relay_id)
// {
//     if (manager == NULL) {
//         return SYSTEM_NULL_PARAMETER; // Handle null manager or relay
//     }
//     if(relay_id < 0) {
//         return SYSTEM_INVALID_PARAMETER; // Invalid relay ID
//     }
//     if(!manager->is_initialized) {
//         return SYSTEM_INVALID_STATE; // Manager is not initialized
//     }
//     if(!emhashmap_contains(&manager->pump_monitors_map, pump_monitor_id)) {
//         return SYSTEM_INVALID_PARAMETER; // Pump monitor not found
//     }
//     monitor_relay_group_t * grp;
//     int event_id = -1; // Initialize event_id to -1
//     grp = get_group_from_tank_monitor_event_map(&manager->pump_monitor_event_map, pump_monitor_id, relay_id,&event_id);
//     if (grp == NULL || event_id < 0) {
//         ESP_LOGE(PUMP_CONTROL_UNIT_TAG,"Failed to find group for pump monitor ID: %d, relay ID: %d", pump_monitor_id, relay_id);
//         return SYSTEM_INVALID_PARAMETER; // Event not found for the given pump monitor and relay ID
//     }

//     // Unsubscribe from the pump monitor event
//     error_type_t err = pump_monitor_unsubscribe_event((pump_monitor_t *)emhashmap_get(&manager->pump_monitors_map, pump_monitor_id), event_id);
//     if (err != SYSTEM_OK) {
//         return err; // Handle error in unsubscribing from the event
//     }

//     // Remove the event from the pump monitor event map
//     if (emhashmap_remove(&manager->pump_monitor_event_map, event_id) == NULL) {
//         return SYSTEM_INVALID_PARAMETER; // Event not found
//     }
//     free(grp); // Free the group memory
//     return SYSTEM_OK;
// }



void callback_handler(void* context,int actuator_id, event_type_t state,int monitor_id){
    // Example callback function for tank monitor events
    ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Tank state changed to: %d for monitor ID: %d", state, monitor_id);
    relay_t *relay = NULL;
    error_type_t err = pump_control_unit_get_relay_by_id((pump_control_unit_t *)context, actuator_id, &relay); 
    if (err != SYSTEM_OK || relay == NULL) {
        ESP_LOGE(PUMP_CONTROL_UNIT_TAG,"Failed to get relay for actuator ID: %d", actuator_id);
        return; // Handle error in getting relay
    }
    // Here you can control the relay based on the state
    if (state == EVENT_TANK_FULL_STATE) {
        ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Turning on relay for actuator ID: %d", actuator_id);
        // Code to turn on the relay
    } else if (state == EVENT_TANK_LOW_STATE) {
        ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"Turning off relay for actuator ID: %d", actuator_id);
        // Code to turn off the relay
    } else {
        ESP_LOGI(PUMP_CONTROL_UNIT_TAG,"No action for state: %d", state);
    }

}
