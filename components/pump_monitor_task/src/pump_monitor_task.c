#include "pump_monitor_task.h"
#include "constant.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "queue_handler_wrapper.h"

static const char *TAG = "PUMP_MONITOR_TASK";



typedef struct
{
    int pump_id;
    int current_sensor_id;
    int pm_id;
    cJSON *pump_config;
    cJSON *current_sensor_config;
} pump_monitor_task_params_t;

typedef struct
{
    pump_t *pump;
    current_sensor_t *cs;
    pump_monitor_t *pm;
    void* context;
    adc_reader_t *adc;
    ads1115_t *ads;
    acs712_sensor_t* acs;
} task_objects_t;

static void pump_event_callback(void *context, int actuator_id, event_type_t event, int pump_monitor_id)
{

    ESP_LOGI(TAG, "Pump state changed to: %d for pump_monitor ID: %d, actuator ID: %d", event, pump_monitor_id, actuator_id);
    
     monitor_event_queue_t pump_monitor = {
        .monitors_id = 1,
        .event = EVENT_PUMP_NORMAL
     };
    error_type_t err = queue_handler_wrapper_send(&pump_monitor);
    if (err != SYSTEM_OK)
    {
        ESP_LOGE(TAG, "failed to send pump monitor event\n.");
    }
    
    if (event == EVENT_PUMP_NORMAL)
    {
        pump_state_machine_state = PUMP_STATE_MACHINE_NORMAL_STATE;
    }
    else if (event == EVENT_PUMP_UNDERCURRENT)
    {
        pump_state_machine_state = PUMP_STATE_MACHINE_UNDERCURRENT_STATE;
    }
    else
    {
        pump_state_machine_state = PUMP_STATE_MACHINE_OVERCURRENT_STATE;
    }
    ESP_LOGI(TAG, "Updated pump_state_machine_state: %d", pump_state_machine_state);
}



static void pump_monitor_task(void *pvParameters)
{
    pump_monitor_task_params_t *params = (pump_monitor_task_params_t *)pvParameters;
    task_objects_t objects = {0};
    int cs_id = params->current_sensor_id;
    

    // Create pump
    cJSON *pump_json = params->pump_config;
    pump_config_t pump_config = {
        .id = cJSON_GetObjectItem(pump_json, "Id")->valueint,
        .make = strdup(cJSON_GetObjectItem(pump_json, "make")->valuestring),
        .power_in_hp = cJSON_GetObjectItem(pump_json, "power_in_hp")->valuedouble,
        .current_rating = cJSON_GetObjectItem(pump_json, "current_rating")->valuedouble};
    objects.pump = pump_create(pump_config);
    if (objects.pump == NULL || pump_init(objects.pump) != SYSTEM_OK)
    {
        ESP_LOGE(TAG, "Failed to create/init pump ID %d", pump_config.id);
        free(pump_config.make);
        cJSON_Delete(params->pump_config);
        cJSON_Delete(params->current_sensor_config);
        free(params);
        vTaskDelete(NULL);
        return;
    }

    // Create current sensor
    cJSON *cs_json = params->current_sensor_config;
    char *interface = cJSON_GetObjectItem(cs_json, "interface")->valuestring;
    acs712_config_t acs_config = {.zero_voltage = 2500}; // Default zero voltage
    error_type_t (*adc_reader_cb)(void *, int *) = NULL;

    if (strncmp(interface, "ADC", 3) == 0)
    {
        // Use default ADC config from constant.h
        adc_reader_config_t local_adc_config = DEFAULT_ADC_CONFIG;
        local_adc_config.adc_channel = (adc_channel_t)(cs_id - 1);
        objects.adc  = adc_reader_create(&local_adc_config);
        if (objects.adc == NULL || adc_reader_init(objects.adc) != SYSTEM_OK)
        {
            ESP_LOGE(TAG, "Failed to create/init ADC for current sensor ID %d", cs_id);
            pump_destroy(&objects.pump);
            free(pump_config.make);
            cJSON_Delete(params->pump_config);
            cJSON_Delete(params->current_sensor_config);
            free(params);
            vTaskDelete(NULL);
            return;
        }
        adc_reader_cb = current_sensor_callback_ac712_adc_read;
        objects.context = (void*)objects.adc;
    }
    else if (strncmp(interface, "I2C", 3) == 0)
    {
        // Use default ADS1115 config from constant.h
        ads1115_config_t local_ads_config = DEFAULT_I2C_CONFIG; 
        objects.ads = ads1115_create(&local_ads_config);
        if (objects.ads == NULL || ads1115_init(objects.ads) != SYSTEM_OK)
        {
            ESP_LOGE(TAG, "Failed to create/init ADS1115 for current sensor ID %d", cs_id);
            pump_destroy(&objects.pump);
            free(pump_config.make);
            cJSON_Delete(params->pump_config);
            cJSON_Delete(params->current_sensor_config);
            free(params);
            vTaskDelete(NULL);
            return;
        }
        // Map current_sensor_id to ADS1115 channel
      
        int channel = CHANNEL_1_CONST;
        switch (channel)
        {
        case 0:
            adc_reader_cb = current_sensor_callback_ads1115_channel0_read;
            break;
        case 1:
            adc_reader_cb = current_sensor_callback_ads1115_channel1_read;
            break;
        case 2:
            adc_reader_cb = current_sensor_callback_ads1115_channel2_read;
            break;
        case 3:
            adc_reader_cb = current_sensor_callback_ads1115_channel3_read;
            break;
        default:
            ESP_LOGE(TAG, "Invalid channel for current sensor ID %d", cs_id);
            pump_destroy(&objects.pump);
            free(pump_config.make);
            cJSON_Delete(params->pump_config);
            cJSON_Delete(params->current_sensor_config);
            free(params);
            vTaskDelete(NULL);
            return;
        }
        objects.context = (void*)objects.ads;
    }
    else
    {
        ESP_LOGE(TAG, "Unknown interface for current sensor ID %d: %s", cs_id, interface);
        pump_destroy(&objects.pump);
        free(pump_config.make);
        cJSON_Delete(params->pump_config);
        cJSON_Delete(params->current_sensor_config);
        free(params);
        vTaskDelete(NULL);
        return;
    }

    acs_config.adc_reader = adc_reader_cb;
    acs_config.context = (void**)&objects.context;
    objects.acs = acs712_create(&acs_config);
    if (objects.acs == NULL || acs712_sensor_init(objects.acs) != SYSTEM_OK)
    {
        ESP_LOGE(TAG, "Failed to create/init ACS712 for current sensor ID %d", cs_id);
        if (objects.adc)
            adc_reader_destroy(&objects.adc);
        if (objects.ads)
            ads1115_destroy(&objects.ads);
        pump_destroy(&objects.pump);
        free(pump_config.make);
        cJSON_Delete(params->pump_config);
        cJSON_Delete(params->current_sensor_config);
        free(params);
        vTaskDelete(NULL);
        return;
    }


    current_sensor_config_t cs_config = {
        .id = cs_id,
        .context = (void **)&objects.acs, // Cast to void** here as well
        .read_current = current_sensor_ac712_read_callback
    };
    objects.cs = current_sensor_create(&cs_config);
    if (objects.cs == NULL || current_sensor_init(objects.cs) != SYSTEM_OK)
    {
        ESP_LOGE(TAG, "Failed to create/init current sensor ID %d", cs_id);
        
        acs712_destroy(&objects.acs);
        if (objects.adc)
            adc_reader_destroy(&objects.adc);
        if (objects.ads)
            ads1115_destroy(&objects.ads);
        pump_destroy(&objects.pump);
        free(pump_config.make);
        cJSON_Delete(params->pump_config);
        cJSON_Delete(params->current_sensor_config);
        free(params);
        vTaskDelete(NULL);
        return;
    }

    // Create pump monitor
    pump_monitor_config_t pm_config = {
        .id = params->pm_id,
        .pump = objects.pump,
        .sensor = objects.cs};
    objects.pm = pump_monitor_create(pm_config);
    if (objects.pm == NULL || pump_monitor_init(objects.pm) != SYSTEM_OK)
    {
        ESP_LOGE(TAG, "Failed to create/init pump monitor ID %d", params->pm_id);
        current_sensor_destroy(&objects.cs);
        acs712_destroy(&objects.acs);
        if (objects.adc)
            adc_reader_destroy(&objects.adc);
        if (objects.ads)
            ads1115_destroy(&objects.ads);
        pump_destroy(&objects.pump);
        free(pump_config.make);
        cJSON_Delete(params->pump_config);
        cJSON_Delete(params->current_sensor_config);
        free(params);
        vTaskDelete(NULL);
        return;
    }

    // Monitoring loop
    pump_monitor_event_hook_t pump_hook = {
        .actuator_id = 1,
        .context = NULL,
        .callback = pump_event_callback
    };
    int event_id = 0;
    error_type_t err = pump_monitor_subscribe_event(objects.pm, &pump_hook, &event_id);
    if (err != SYSTEM_OK)
    {
        ESP_LOGE(TAG, "Failed to subscribe monitor\n");
    }
    
    ESP_LOGI(TAG, "Pump monitor task started for PM ID %d (Pump ID %d, CS ID %d)",
             params->pm_id, params->pump_id, params->current_sensor_id);
    while (1)
    {
        err = pump_monitor_check_current(objects.pm);
        if (err != SYSTEM_OK)
        {
            ESP_LOGE(TAG, "Failed to check current for pump monitor ID %d", params->pm_id);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Check every 1 second
    }
}

esp_err_t start_pump_monitor_task(const char *json_str, size_t json_size, TaskHandle_t *task_handle)
{
    // Parse JSON
    cJSON *json = cJSON_ParseWithLength(json_str, json_size);
    if (json == NULL)
    {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return ESP_FAIL; 
    }

    cJSON *pcus_json = cJSON_GetObjectItem(json, "Pump Control Units");
    cJSON *pcu = cJSON_GetArrayItem(pcus_json, 0); // Use first PCU
    cJSON *pm_array = cJSON_GetObjectItem(pcu, "pump_monitors");
    cJSON *pm_item = cJSON_GetArrayItem(pm_array, 0); // Use first pump monitor
    int pm_id = cJSON_GetObjectItem(pm_item, "Id")->valueint;
    int pump_id = cJSON_GetObjectItem(pm_item, "Pump Id")->valueint;
    int cs_id = cJSON_GetObjectItem(pm_item, "Current Sensor Id")->valueint;

    // Find pump config
    cJSON *pumps_array = cJSON_GetObjectItem(pcu, "pumps");
    cJSON *pump_json = NULL;
    for (int i = 0; i < cJSON_GetArraySize(pumps_array); i++)
    {
        cJSON *p = cJSON_GetArrayItem(pumps_array, i);
        if (cJSON_GetObjectItem(p, "Id")->valueint == pump_id)
        {
            pump_json = cJSON_Duplicate(p, 1);
            break;
        }
    }
    if (!pump_json)
    {
        ESP_LOGE(TAG, "Pump ID %d not found for PM ID %d", pump_id, pm_id);
        cJSON_Delete(json);
        return ESP_FAIL;
    }

    // Find current sensor config
    cJSON *cs_array = cJSON_GetObjectItem(pcu, "current_sensor");
    cJSON *cs_json = NULL;
    for (int i = 0; i < cJSON_GetArraySize(cs_array); i++)
    {
        cJSON *cs = cJSON_GetArrayItem(cs_array, i);
        if (cJSON_GetObjectItem(cs, "Id")->valueint == cs_id)
        {
            cs_json = cJSON_Duplicate(cs, 1);
            break;
        }
    }
    if (!cs_json)
    {
        ESP_LOGE(TAG, "Current sensor ID %d not found for PM ID %d", cs_id, pm_id);
        cJSON_Delete(pump_json);
        cJSON_Delete(json);
        return ESP_FAIL;
    }

    // Create task parameters
    pump_monitor_task_params_t *params = malloc(sizeof(pump_monitor_task_params_t));
    if (!params)
    {
        ESP_LOGE(TAG, "Failed to allocate task params for PM ID %d", pm_id);
        cJSON_Delete(pump_json);
        cJSON_Delete(cs_json);
        cJSON_Delete(json);
        return ESP_FAIL;
    }
    params->pump_id = pump_id;
    params->current_sensor_id = cs_id;
    params->pm_id = pm_id;
    params->pump_config = pump_json;
    params->current_sensor_config = cs_json;

    // Start task
    char task_name[32];
    snprintf(task_name, sizeof(task_name), "pump_monitor_task_%d", pm_id);
    BaseType_t result = xTaskCreate(pump_monitor_task, task_name, 8192, params, 5, task_handle);
    if (result != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create task for pump monitor ID %d", pm_id);
        cJSON_Delete(pump_json);
        cJSON_Delete(cs_json);
        free(params);
        cJSON_Delete(json);
        return ESP_FAIL;
    }

    cJSON_Delete(json);
    return ESP_OK;
}



