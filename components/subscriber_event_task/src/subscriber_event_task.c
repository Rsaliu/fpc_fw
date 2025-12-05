#include <subscriber_event_task.h>
#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <common_headers.h>
#include "esp_log.h"

static const char *TAG = "SUBSCRIBER_EVENT";

tank_state_machine_state_t tank_state_machine_state = TANK_STATE_MACHINE_NORMAL_STATE;
void dummy_event_callback(void* context,int actuator_id, event_type_t event,int monitor_id) {
    // Example callback function for tank monitor events
    ESP_LOGI(TAG,"Tank state changed to: %d for monitor ID: %d, actuator ID: %d\n", event, monitor_id,actuator_id);
    
    // Simulate changing the state based on the event
    if (event == EVENT_TANK_FULL_STATE) {
        tank_state_machine_state = TANK_STATE_MACHINE_FULL_STATE;
    } else if (event == EVENT_TANK_LOW_STATE) {
        tank_state_machine_state = TANK_STATE_MACHINE_LOW_STATE;
    } else {
        tank_state_machine_state = TANK_STATE_MACHINE_NORMAL_STATE;
    }
}

void subscriber_event_task(void *Pvparameter)
{
    subcriber_event_task_t *subscriber = (subcriber_event_task_t *)Pvparameter;
    if (subscriber == NULL || subscriber->monitor == NULL)
    {
        ESP_LOGE(TAG, "Invalid subscriber, monitor or event_hook\n ");
    }

    error_type_t err = tank_monitor_subscribe_state_event(subscriber->monitor, &subscriber->event_hook, &subscriber->event_id);
    if (err == SYSTEM_OK)
    {
        ESP_LOGI(TAG, "subscribtion to monitor and Event_id= %d is sucessfull",subscriber->event_id);
    }
    else
    {
        ESP_LOGE(TAG, "failed to subscribe to an event");
    }

    while (1)
    {
         ESP_LOGI(TAG,"current tank_state_machine_state is: %d\n",tank_state_machine_state);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
