#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "pump_monitor_task.h"

static const char *TAG = "PUMP_MONITOR_TASK";

static const char *json_config =
    "{\n"
    "  \"Site Id\": \"Site1\",\n"
    "  \"Device Id\": \"Device1\",\n"
    "  \"Pump Control Units\": [\n"
    "    {\n"
    "      \"Id\": 1,\n"
    "      \"pumps\": [\n"
    "        {\n"
    "          \"Id\": 1,\n"
    "          \"make\": \"PumpA\",\n"
    "          \"power_in_hp\": 1.5,\n"
    "          \"current_rating\": 5.0\n"
    "        }\n"
    "      ],\n"
    "      \"current_sensor\": [\n"
    "        {\n"
    "          \"Id\": 1,\n"
    "          \"interface\": \"ADC\",\n"
    "          \"make\": \"ACS712\",\n"
    "          \"max_current\": 30\n"
    "        }\n"
    "      ],\n"
    "      \"pump_monitors\": [\n"
    "        {\n"
    "          \"Id\": 1,\n"
    "          \"Pump Id\": 1,\n"
    "          \"Current Sensor Id\": 1\n"
    "        }\n"
    "      ]\n"
    "    }\n"
    "  ],\n"
    "  \"mappings\": []\n"
    "}";

void app_main(void)
{

    printf("Hello world!\n");

    ESP_LOGI(TAG, "Free heap: %u bytes", (unsigned)esp_get_free_heap_size());

    TaskHandle_t task_handle;

    esp_err_t err = start_pump_monitor_task(json_config, strlen(json_config), &task_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start pump monitor task");
        return;
    }
    ESP_LOGI(TAG, "Pump monitor task started successfully!");

    while (1)
    {
        ESP_LOGI(TAG, "Main loop running, free heap: %lu bytes", esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
