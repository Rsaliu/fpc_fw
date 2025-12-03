#include "webserver_task.h"
#include "web_server_setup.h"
#include "esp_log.h"

static const char *TAG = "WEBSERVER_TASK";

static TaskHandle_t webserver_task_handle = NULL;

#define TASK_STACK_SIZE (4096)
#define TASK_PRIORITY (5)

static void webserver_task_function(void *pvParameters)
{
    const webserver_config_t *config = (const webserver_config_t *)pvParameters;

    webserver_t *server = webserver_create(config);
    if (server == NULL){
        ESP_LOGE(TAG, "Failed to create webserver instance");
        vTaskDelete(NULL);
        return;
    }

    if (webserver_init(server) != SYSTEM_OK){
        ESP_LOGE(TAG, "Failed to initialize webserver");
        webserver_destroy(&server);
        vTaskDelete(NULL);
        return;
    }

      if (webserver_start(server) != SYSTEM_OK) {
        ESP_LOGE(TAG, "Failed to start webserver");
        webserver_deinit(server);
        webserver_destroy(&server);
        vTaskDelete(NULL);
        return;
    }
     rest_server_context_t *context_ptr = NULL;
    if (webserver_get_context(server, &context_ptr) != SYSTEM_OK) {
        ESP_LOGE(TAG, "Failed to retrieve webserver context");
        webserver_deinit(server);
        webserver_destroy(&server);
        vTaskDelete(NULL);
        return;
    }

    if (setup_web_handlers(server, context_ptr) != SYSTEM_OK) {
        ESP_LOGE(TAG, "setup_web_handlers failed");
        webserver_deinit(server);
        webserver_destroy(&server);
        vTaskDelete(NULL);
        return;
    }


    ESP_LOGI(TAG, "Webserver task started successfully");

    while (1){
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

error_type_t webserver_task_start(const webserver_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "Invalid config provided");
        return SYSTEM_NULL_PARAMETER;
    }
    if (webserver_task_handle != NULL){
        ESP_LOGW(TAG, "Webserver task already running");
        return SYSTEM_INVALID_STATE;
    }

    BaseType_t ret = xTaskCreate(
        webserver_task_function,
        "webserver_task",
        TASK_STACK_SIZE,
        (void *)config,
        TASK_PRIORITY,
        &webserver_task_handle
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create webserver task");
        return SYSTEM_OPERATION_FAILED;
    }

    ESP_LOGI(TAG, "Webserver task creation initiated");
    return SYSTEM_OK;
}

bool webserver_task_is_running(void)
{
    return (webserver_task_handle != NULL);
}