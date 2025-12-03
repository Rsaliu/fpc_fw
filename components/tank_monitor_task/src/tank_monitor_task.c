#include <tank_monitor_task.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "queue_handler_wrapper.h"
#include "relay_driver.h"

static const char *TAG = "TANK_MONITOR_TASK";

static void tank_event_callback(void *context, int actuator_id, event_type_t event, int monitor_id)
{
  // Example callback function for tank monitor events
  ESP_LOGI(TAG, "Tank state changed to: %d for monitor ID: %d, actuator ID: %d\n", (int)event, monitor_id, actuator_id);
  pump_control_unit_t *pcu = (pump_control_unit_t *)context;
  relay_t *relay = NULL;
  error_type_t err = pump_control_unit_get_relay_by_id(pcu, actuator_id, &relay);
  if (err != SYSTEM_OK)
  {
    ESP_LOGE(TAG, "failed to get relay id\n");
  }

  monitor_event_queue_t queue = {
      .actuator_id = actuator_id,
      .context = context,
      .monitor_id = monitor_id,
      .event = event};
  err = queue_handler_wrapper_send(&queue);
  if (err != SYSTEM_OK)
  {
    ESP_LOGE(TAG, "failed to send a queue\n.");
  }
}

void tank_monitor_task(void *pvParameter)
{
  tank_monitor_task_config_t *config = (tank_monitor_task_config_t *)pvParameter;

  if (config == NULL || config->tank_monitor == NULL)
  {
    ESP_LOGE(TAG, "Invalide config or invalide tank monitor array");
  }

  // Subscribe once before the loop
  for (int i = 0; i < config->tank_monitor_count; i++)
  {
    relay_config_t relay_cfg;
    error_type_t err = relay_get_config(config->relay[i], &relay_cfg);
    if (err != SYSTEM_OK)
    {
      ESP_LOGE(TAG, "failed to get relay config\n.");
    }

    tank_monitor_event_hook_t hook = {
        .context = config->pcu,
        .actuator_id = relay_cfg.id,
        .callback  = tank_event_callback};

    int event_id;
    tank_monitor_subscribe_event(config->tank_monitor[i], &hook, &event_id);
    ESP_LOGI(TAG, "Subscribed to monitor[%d] relay_id [%d] with event_id %d", i, relay_cfg.id, event_id);
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
      tank_monitor_check_level(config->tank_monitor[i]);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
