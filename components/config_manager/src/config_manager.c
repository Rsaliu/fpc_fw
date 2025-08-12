#include "config_manager.h"
#include <esp_log.h>
#include <cJSON.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "CONFIG_MANAGER";

static esp_err_t (*read_config_json_func)(const char *path, char **buffer, size_t *size) = NULL;

esp_err_t read_config_json(const char *path, char **buffer, size_t *size)
{
    if (path == NULL || buffer == NULL || size == NULL)
    {
        ESP_LOGE(TAG, "Invalid arguments");
        return ESP_ERR_INVALID_ARG;
    }

    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file: %s", path);
        return ESP_FAIL;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *buffer = (char *)malloc(*size + 1);
    if (*buffer == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for file contents");
        fclose(file);
        return ESP_ERR_NO_MEM;
    }

    size_t read_size = fread(*buffer, 1, *size, file);
    if (read_size != *size)
    {
        ESP_LOGE(TAG, "Failed to read file: %s", path);
        free(*buffer);
        *buffer = NULL;
        fclose(file);
        return ESP_FAIL;
    }

    (*buffer)[*size] = '\0';
    fclose(file);
    ESP_LOGI(TAG, "Successfully read file: %s (%zu bytes)", path, *size);
    return ESP_OK;
}

static esp_err_t validate_json_structure(const cJSON *json)
{
    if (!cJSON_IsObject(json))
    {
        ESP_LOGE(TAG, "Root is not an object");
        return ESP_FAIL;
    }

  

    cJSON *site_id = cJSON_GetObjectItem(json, "Site Id");
    if (!site_id || !cJSON_IsString(site_id) || site_id->valuestring[0] == '\0')
    {
        ESP_LOGE(TAG, "\"Site Id\" is missing, not a string, or empty");
        return ESP_FAIL;
    }


    cJSON *device_id = cJSON_GetObjectItem(json, "Device Id");
    if (!device_id || !cJSON_IsString(device_id) || device_id->valuestring[0] == '\0')
    {
        ESP_LOGE(TAG, "\"Device Id\" is missing, not a string, or empty");
        return ESP_FAIL;
    }

    cJSON *pump_control_units = cJSON_GetObjectItem(json, "Pump Control Units");
    if (!cJSON_IsArray(pump_control_units))
    {
        ESP_LOGE(TAG, "\"Pump Control Units\" is missing or not an array");
        return ESP_FAIL;
    }

    cJSON *pcu_item;
    cJSON_ArrayForEach(pcu_item, pump_control_units)
    {
        if (!cJSON_IsObject(pcu_item))
        {
            ESP_LOGE(TAG, "Item in \"Pump Control Units\" is not an object");
            return ESP_FAIL;
        }

        cJSON *id = cJSON_GetObjectItem(pcu_item, "Id");
        if (!cJSON_IsNumber(id) || id->valueint != (int)id->valuedouble)
        {
            ESP_LOGE(TAG, "\"Id\" in Pump Control Units is missing or not an integer");
            return ESP_FAIL;
        }

        cJSON *tank_monitors = cJSON_GetObjectItem(pcu_item, "tank_monitors");
        if (!cJSON_IsArray(tank_monitors))
        {
            ESP_LOGE(TAG, "\"tank_monitors\" is missing or not an array");
            return ESP_FAIL;
        }
        cJSON *tm_item;
        cJSON_ArrayForEach(tm_item, tank_monitors)
        {
            if (!cJSON_IsObject(tm_item))
            {
                ESP_LOGE(TAG, "Item in \"tank_monitors\" is not an object");
                return ESP_FAIL;
            }
            cJSON *tm_id = cJSON_GetObjectItem(tm_item, "Id");
            if (!cJSON_IsNumber(tm_id) || tm_id->valueint != (int)tm_id->valuedouble)
            {
                ESP_LOGE(TAG, "\"Id\" in tank_monitors is missing or not an integer");
                return ESP_FAIL;
            }
            cJSON *tank_id = cJSON_GetObjectItem(tm_item, "tank Id");
            if (!cJSON_IsNumber(tank_id) || tank_id->valueint != (int)tank_id->valuedouble)
            {
                ESP_LOGE(TAG, "\"tank Id\" in tank_monitors is missing or not an integer");
                return ESP_FAIL;
            }
            cJSON *level_sensor_id = cJSON_GetObjectItem(tm_item, "level sensor Id");
            if (!cJSON_IsNumber(level_sensor_id) || level_sensor_id->valueint != (int)level_sensor_id->valuedouble)
            {
                ESP_LOGE(TAG, "\"level sensor Id\" in tank_monitors is missing or not an integer");
                return ESP_FAIL;
            }
        }

        cJSON *pump_monitors = cJSON_GetObjectItem(pcu_item, "pump_monitors");
        if (!cJSON_IsArray(pump_monitors))
        {
            ESP_LOGE(TAG, "\"pump_monitors\" is missing or not an array");
            return ESP_FAIL;
        }
        cJSON *pm_item;
        cJSON_ArrayForEach(pm_item, pump_monitors)
        {
            if (!cJSON_IsObject(pm_item))
            {
                ESP_LOGE(TAG, "Item in \"pump_monitors\" is not an object");
                return ESP_FAIL;
            }
            cJSON *pm_id = cJSON_GetObjectItem(pm_item, "Id");
            if (!cJSON_IsNumber(pm_id) || pm_id->valueint != (int)pm_id->valuedouble)
            {
                ESP_LOGE(TAG, "\"Id\" in pump_monitors is missing or not an integer");
                return ESP_FAIL;
            }
            cJSON *pump_id = cJSON_GetObjectItem(pm_item, "Pump Id");
            if (!cJSON_IsNumber(pump_id) || pump_id->valueint != (int)pump_id->valuedouble)
            {
                ESP_LOGE(TAG, "\"Pump Id\" in pump_monitors is missing or not an integer");
                return ESP_FAIL;
            }
            cJSON *current_sensor_id = cJSON_GetObjectItem(pm_item, "Current Sensor Id");
            if (!cJSON_IsNumber(current_sensor_id) || current_sensor_id->valueint != (int)current_sensor_id->valuedouble)
            {
                ESP_LOGE(TAG, "\"Current Sensor Id\" in pump_monitors is missing or not an integer");
                return ESP_FAIL;
            }
        }

        cJSON *tanks = cJSON_GetObjectItem(pcu_item, "tanks");
        if (!cJSON_IsArray(tanks))
        {
            ESP_LOGE(TAG, "\"tanks\" is missing or not an array");
            return ESP_FAIL;
        }
        cJSON *tank_item;
        cJSON_ArrayForEach(tank_item, tanks)
        {
            if (!cJSON_IsObject(tank_item))
            {
                ESP_LOGE(TAG, "Item in \"tanks\" is not an object");
                return ESP_FAIL;
            }
            cJSON *tank_id = cJSON_GetObjectItem(tank_item, "Id");
            if (!cJSON_IsNumber(tank_id) || tank_id->valueint != (int)tank_id->valuedouble)
            {
                ESP_LOGE(TAG, "\"Id\" in tanks is missing or not an integer");
                return ESP_FAIL;
            }
            cJSON *capacity = cJSON_GetObjectItem(tank_item, "capacity In Litres");
            if (!cJSON_IsNumber(capacity))
            {
                ESP_LOGE(TAG, "\"capacity In Litres\" in tanks is missing or not a number");
                return ESP_FAIL;
            }
            cJSON *shape = cJSON_GetObjectItem(tank_item, "shape");
            if (!cJSON_IsString(shape))
            {
                ESP_LOGE(TAG, "\"shape\" in tanks is missing or not a string");
                return ESP_FAIL;
            }
            if (strcmp(shape->valuestring, "RECTANGLE") != 0 && strcmp(shape->valuestring, "CYLINDER") != 0)
            {
                ESP_LOGE(TAG, "\"shape\" in tanks is invalid: %s", shape->valuestring);
                return ESP_FAIL;
            }
            cJSON *height = cJSON_GetObjectItem(tank_item, "height In cm");
            if (!cJSON_IsNumber(height))
            {
                ESP_LOGE(TAG, "\"height In cm\" in tanks is missing or not a number");
                return ESP_FAIL;
            }
            cJSON *full_level = cJSON_GetObjectItem(tank_item, "full level mm");
            if (!cJSON_IsNumber(full_level) || full_level->valueint != (int)full_level->valuedouble)
            {
                ESP_LOGE(TAG, "\"full level mm\" in tanks is missing or not an integer");
                return ESP_FAIL;
            }
            cJSON *low_level = cJSON_GetObjectItem(tank_item, "low level mm");
            if (!cJSON_IsNumber(low_level) || low_level->valueint != (int)low_level->valuedouble)
            {
                ESP_LOGE(TAG, "\"low level mm\" in tanks is missing or not an integer");
                return ESP_FAIL;
            }
        }

        cJSON *pumps = cJSON_GetObjectItem(pcu_item, "pumps");
        if (!cJSON_IsArray(pumps))
        {
            ESP_LOGE(TAG, "\"pumps\" is missing or not an array");
            return ESP_FAIL;
        }
        cJSON *pump_item;
        cJSON_ArrayForEach(pump_item, pumps)
        {
            if (!cJSON_IsObject(pump_item))
            {
                ESP_LOGE(TAG, "Item in \"pumps\" is not an object");
                return ESP_FAIL;
            }
            cJSON *pump_id = cJSON_GetObjectItem(pump_item, "Id");
            if (!cJSON_IsNumber(pump_id) || pump_id->valueint != (int)pump_id->valuedouble)
            {
                ESP_LOGE(TAG, "\"Id\" in pumps is missing or not an integer");
                return ESP_FAIL;
            }
            cJSON *make = cJSON_GetObjectItem(pump_item, "make");
            if (!cJSON_IsString(make))
            {
                ESP_LOGE(TAG, "\"make\" in pumps is missing or not a string");
                return ESP_FAIL;
            }
            cJSON *power = cJSON_GetObjectItem(pump_item, "power_in_hp");
            if (!cJSON_IsNumber(power))
            {
                ESP_LOGE(TAG, "\"power_in_hp\" in pumps is missing or not a number");
                return ESP_FAIL;
            }
            cJSON *current_rating = cJSON_GetObjectItem(pump_item, "current_rating");
            if (!cJSON_IsNumber(current_rating))
            {
                ESP_LOGE(TAG, "\"current_rating\" in pumps is missing or not a number");
                return ESP_FAIL;
            }
        }

        cJSON *relays = cJSON_GetObjectItem(pcu_item, "Relays");
        if (!cJSON_IsArray(relays))
        {
            ESP_LOGE(TAG, "\"Relays\" is missing or not an array");
            return ESP_FAIL;
        }
        cJSON *relay_item;
        cJSON_ArrayForEach(relay_item, relays)
        {
            if (!cJSON_IsObject(relay_item))
            {
                ESP_LOGE(TAG, "Item in \"Relays\" is not an object");
                return ESP_FAIL;
            }
            cJSON *relay_id = cJSON_GetObjectItem(relay_item, "Id");
            if (!cJSON_IsNumber(relay_id) || relay_id->valueint != (int)relay_id->valuedouble)
            {
                ESP_LOGE(TAG, "\"Id\" in Relays is missing or not an integer");
                return ESP_FAIL;
            }
            cJSON *pin_number = cJSON_GetObjectItem(relay_item, "Relay_pin_number");
            if (!cJSON_IsNumber(pin_number) || pin_number->valueint != (int)pin_number->valuedouble)
            {
                ESP_LOGE(TAG, "\"Relay_pin_number\" in Relays is missing or not an integer");
                return ESP_FAIL;
            }
        }

        cJSON *current_sensors = cJSON_GetObjectItem(pcu_item, "current_sensor");
        if (!cJSON_IsArray(current_sensors))
        {
            ESP_LOGE(TAG, "\"current_sensor\" is missing or not an array");
            return ESP_FAIL;
        }
        cJSON *cs_item;
        cJSON_ArrayForEach(cs_item, current_sensors)
        {
            if (!cJSON_IsObject(cs_item))
            {
                ESP_LOGE(TAG, "Item in \"current_sensor\" is not an object");
                return ESP_FAIL;
            }
            cJSON *cs_id = cJSON_GetObjectItem(cs_item, "Id");
            if (!cJSON_IsNumber(cs_id) || cs_id->valueint != (int)cs_id->valuedouble)
            {
                ESP_LOGE(TAG, "\"Id\" in current_sensor is missing or not an integer");
                return ESP_FAIL;
            }
            cJSON *interface = cJSON_GetObjectItem(cs_item, "interface");
            if (!cJSON_IsString(interface))
            {
                ESP_LOGE(TAG, "\"interface\" in current_sensor is missing or not a string");
                return ESP_FAIL;
            }
            cJSON *cs_make = cJSON_GetObjectItem(cs_item, "make");
            if (!cJSON_IsString(cs_make))
            {
                ESP_LOGE(TAG, "\"make\" in current_sensor is missing or not a string");
                return ESP_FAIL;
            }
            cJSON *max_current = cJSON_GetObjectItem(cs_item, "max_current");
            if (!cJSON_IsNumber(max_current) || max_current->valueint != (int)max_current->valuedouble)
            {
                ESP_LOGE(TAG, "\"max_current\" in current_sensor is missing or not an integer");
                return ESP_FAIL;
            }
        }

        cJSON *level_sensor = cJSON_GetObjectItem(pcu_item, "Level_sensor");
        if (!cJSON_IsObject(level_sensor))
        {
            ESP_LOGE(TAG, "\"Level_sensor\" is missing or not an object");
            return ESP_FAIL;
        }
        cJSON *ls_id = cJSON_GetObjectItem(level_sensor, "Id");
        if (!cJSON_IsNumber(ls_id) || ls_id->valueint != (int)ls_id->valuedouble)
        {
            ESP_LOGE(TAG, "\"Id\" in Level_sensor is missing or not an integer");
            return ESP_FAIL;
        }
        cJSON *interface = cJSON_GetObjectItem(level_sensor, "interface");
        if (!cJSON_IsString(interface))
        {
            ESP_LOGE(TAG, "\"interface\" in Level_sensor is missing or not a string");
            return ESP_FAIL;
        }
        cJSON *sensor_add = cJSON_GetObjectItem(level_sensor, "sensor_add");
        if (!cJSON_IsNumber(sensor_add) || sensor_add->valueint != (int)sensor_add->valuedouble)
        {
            ESP_LOGE(TAG, "\"sensor_add\" in Level_sensor is missing or not an integer");
            return ESP_FAIL;
        }
        cJSON *protocol = cJSON_GetObjectItem(level_sensor, "protocol");
        if (!cJSON_IsString(protocol))
        {
            ESP_LOGE(TAG, "\"protocol\" in Level_sensor is missing or not a string");
            return ESP_FAIL;
        }
    }

    cJSON *mappings = cJSON_GetObjectItem(json, "mappings");
    if (!cJSON_IsArray(mappings))
    {
        ESP_LOGE(TAG, "\"mappings\" is missing or not an array");
        return ESP_FAIL;
    }
    cJSON *mapping_item;
    cJSON_ArrayForEach(mapping_item, mappings)
    {
        if (!cJSON_IsObject(mapping_item))
        {
            ESP_LOGE(TAG, "Item in \"mappings\" is not an object");
            return ESP_FAIL;
        }
        cJSON *tank_monitor_ids = cJSON_GetObjectItem(mapping_item, "tank_monitor_ids");
        if (!cJSON_IsArray(tank_monitor_ids))
        {
            ESP_LOGE(TAG, "\"tank_monitor_ids\" is missing or not an array");
            return ESP_FAIL;
        }
        cJSON *tmid_item;
        cJSON_ArrayForEach(tmid_item, tank_monitor_ids)
        {
            if (!cJSON_IsNumber(tmid_item) || tmid_item->valueint != (int)tmid_item->valuedouble)
            {
                ESP_LOGE(TAG, "Item in \"tank_monitor_ids\" is not an integer");
                return ESP_FAIL;
            }
        }

        cJSON *pump_monitor_ids = cJSON_GetObjectItem(mapping_item, "pump_monitor_ids");
        if (!cJSON_IsArray(pump_monitor_ids))
        {
            ESP_LOGE(TAG, "\"pump_monitor_ids\" is missing or not an array");
            return ESP_FAIL;
        }
        cJSON *pmid_item;
        cJSON_ArrayForEach(pmid_item, pump_monitor_ids)
        {
            if (!cJSON_IsNumber(pmid_item) || pmid_item->valueint != (int)pmid_item->valuedouble)
            {
                ESP_LOGE(TAG, "Item in \"pump_monitor_ids\" is not an integer");
                return ESP_FAIL;
            }
        }

        cJSON *relay_ids = cJSON_GetObjectItem(mapping_item, "relay_ids");
        if (!cJSON_IsArray(relay_ids))
        {
            ESP_LOGE(TAG, "\"relay_ids\" is missing or not an array");
            return ESP_FAIL;
        }
        cJSON *rid_item;
        cJSON_ArrayForEach(rid_item, relay_ids)
        {
            if (!cJSON_IsNumber(rid_item) || rid_item->valueint != (int)rid_item->valuedouble)
            {
                ESP_LOGE(TAG, "Item in \"relay_ids\" is not an integer");
                return ESP_FAIL;
            }
        }
    }

    ESP_LOGI(TAG, "JSON validation successful");
    return ESP_OK;
}

esp_err_t validate_config_json(const char *path) {
    if (path == NULL) {
        ESP_LOGE(TAG, "Path is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    char *buffer = NULL;
    size_t size = 0;


    if (read_config_json_func == NULL) {
        read_config_json_func = read_config_json;
    }

    esp_err_t ret = read_config_json_func(path, &buffer, &size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read file: %s", esp_err_to_name(ret));
        return ret;
    }

    cJSON *json = cJSON_Parse(buffer);
    free(buffer);

    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGE(TAG, "Invalid JSON: error before: %s", error_ptr);
        } else {
            ESP_LOGE(TAG, "Invalid JSON");
        }
        return ESP_FAIL;
    }

    ret = validate_json_structure(json);
    cJSON_Delete(json); 

    return ret;
}

// For test code to override file reading
void config_manager_set_reader(esp_err_t (*func)(const char *, char **, size_t *)) {
    read_config_json_func = func;
}
