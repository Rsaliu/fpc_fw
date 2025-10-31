#include <tank_monitor_task.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "subscriber_event_task.h"

static const char *TAG = "TANK_MONITOR_TASK";

tank_state_machine_state_t tank_state_machine_state = TANK_STATE_MACHINE_NORMAL_STATE;
static void event_callback(void *context, int actuator_id, event_type_t event, int monitor_id)
{
  // Example callback function for tank monitor events
  ESP_LOGI(TAG, "Tank state changed to: %d for monitor ID: %d, actuator ID: %d\n", event, monitor_id, actuator_id);

  monitor_event_queue_t tank_monitor_event = {
      .event = EVENT_TANK_NORMAL_STATE,
      .monitors_id = 1};

  error_type_t err = queue_handler_wrapper_send(&tank_monitor_event);

  if (err != SYSTEM_OK)
  {
    ESP_LOGE(TAG, "failed to send tank monitor event\n.");
  }

  // Simulate changing the state based on the event
  if (event == EVENT_TANK_FULL_STATE)
  {
    tank_state_machine_state = TANK_STATE_MACHINE_FULL_STATE;
  }
  else if (event == EVENT_TANK_LOW_STATE)
  {
    tank_state_machine_state = TANK_STATE_MACHINE_LOW_STATE;
  }
  else
  {
    tank_state_machine_state = TANK_STATE_MACHINE_NORMAL_STATE;
  }
}

void tank_monitor_task(void *pvParameter)
{
  tank_monitor_task_config_t *config = (tank_monitor_task_config_t *)pvParameter;

  if (config == NULL || config->tank_monitor == NULL)
  {
    ESP_LOGE(TAG, "Invalide config or invalide tank monitor array");
  }

  // subscribe to an event once for each tank monitor
  ESP_LOGI(TAG, "Starting tank monitor subscription setup...");
  ESP_LOGI(TAG, "Number of tank monitors: %d", config->tank_monitor_count);
  for (int i = 0; i < config->tank_monitor_count; i++)
  {
    if (config->tank_monitor[i] != NULL)
    {
      ESP_LOGI(TAG, "Monitor[%d]  subscribe to an event", i);
    }

    tank_monitor_event_hook_t event = {
        .actuator_id = 1,
        .callback = event_callback,
        .context = NULL};

    int event_id = 0;
    error_type_t err = tank_monitor_subscribe_event(config->tank_monitor[i], &event, &event_id);

    if (err != SYSTEM_OK)
    {
      ESP_LOGE(TAG, "Failed to subscribe monitor[%d]", i);
    }
  }

  ESP_LOGI(TAG, "entering the task monitor loop......\n");
  while (1)
  {
    for (int i = 0; i < config->tank_monitor_count; i++)
    {
      if (config->tank_monitor[i] == NULL)
      {
        ESP_LOGE(TAG, "Monitor[%d] is NULL", i);
        
      }
      ESP_LOGI(TAG, "number of monitor[%d]\n", i);
      tank_monitor_check_level(config->tank_monitor[i]);
    }
  }
  vTaskDelay(pdMS_TO_TICKS(1000));
}
