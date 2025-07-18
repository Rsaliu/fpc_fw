#include "wifi_hotspot.h"
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_mac.h"



static const char *TAG = "wifi_hotspot";

struct wifi_hotspot_t
{
    wifi_hotspot_config_t config;
    bool is_initialized;
    bool is_running;
};


static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d, reason=%d", MAC2STR(event->mac), event->aid, event->reason);
    }
}

static esp_err_t map_auth_mode(wifi_hotspot_auth_mode_t auth_mode, wifi_auth_mode_t *esp_auth_mode)
{
    switch (auth_mode)
    {
    case WIFI_HOTSPOT_AUTH_OPEN:
        *esp_auth_mode = WIFI_AUTH_OPEN;
        break;
    case WIFI_HOTSPOT_AUTH_WPA2_PSK:
        *esp_auth_mode = WIFI_AUTH_WPA2_PSK;
        break;
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
    case WIFI_HOTSPOT_AUTH_WPA3_PSK:
        *esp_auth_mode = WIFI_AUTH_WPA3_PSK;
        break;
#endif
    default:
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

wifi_hotspot_t *wifi_hotspot_create(wifi_hotspot_config_t config)
{

    if (config.ssid[0] == '\0' || strlen(config.ssid) > 31)
    {
        ESP_LOGE(TAG, "Invalid SSID");
        return NULL;
    }
    if (config.auth_mode != WIFI_HOTSPOT_AUTH_OPEN && (strlen(config.password) < 8 || strlen(config.password) > 63))
    {
        ESP_LOGE(TAG, "Invalid password length");
        return NULL;
    }
    if (config.channel < 1 || config.channel > 13)
    {
        ESP_LOGE(TAG, "Invalid channel");
        return NULL;
    }
    if (config.max_connections < 1 || config.max_connections > 10)
    {
        ESP_LOGE(TAG, "Invalid max connections");
        return NULL;
    }
    wifi_auth_mode_t esp_auth_mode;
    if (map_auth_mode(config.auth_mode, &esp_auth_mode) != ESP_OK)
    {
        ESP_LOGE(TAG, "Invalid auth mode");
        return NULL;
    }

    wifi_hotspot_t *wifi_hotspot = (wifi_hotspot_t *)malloc(sizeof(wifi_hotspot_t));
    if (!wifi_hotspot)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for hotspot");
        return NULL;
    }

    wifi_hotspot->config = config;
    wifi_hotspot->is_initialized = false;
    wifi_hotspot->is_running = false;

    return wifi_hotspot;
}

error_type_t wifi_hotspot_init(wifi_hotspot_t *wifi_hotspot)
{
    if (!wifi_hotspot) {
        ESP_LOGE(TAG, "Null hotspot pointer");
        return SYSTEM_NULL_PARAMETER;
    }
    if (wifi_hotspot->is_initialized) {
        ESP_LOGE(TAG, "Hotspot already initialized");
        return SYSTEM_INVALID_STATE;
    }

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(ret));
        return SYSTEM_OPERATION_FAILED;
    }


    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

   
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    
    wifi_hotspot->is_initialized = true;
    ESP_LOGI(TAG, "Hotspot initialized");
    return SYSTEM_OK;
}
error_type_t wifi_hotspot_deinit(wifi_hotspot_t *wifi_hotspot)
{
    if (!wifi_hotspot)
    {
        ESP_LOGE(TAG, "Null hotspot pointer");
        return SYSTEM_NULL_PARAMETER;
    }
    if (!wifi_hotspot->is_initialized)
    {
        ESP_LOGE(TAG, "Hotspot not initialized");
        return SYSTEM_INVALID_STATE;
    }

    if (wifi_hotspot->is_running)
    {
        ESP_ERROR_CHECK(esp_wifi_stop());
        wifi_hotspot->is_running = false;
    }

    ESP_ERROR_CHECK(esp_wifi_deinit());

    ESP_ERROR_CHECK(esp_event_loop_delete_default());
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (netif)
    {
        esp_netif_destroy(netif);
    }

    wifi_hotspot->is_initialized = false;
    ESP_LOGI(TAG, "Hotspot deinitialized");
    return SYSTEM_OK;
}

error_type_t wifi_hotspot_destroy(wifi_hotspot_t **wifi_hotspot)
{
    if (!wifi_hotspot || !*wifi_hotspot)
    {
        ESP_LOGE(TAG, "Null hotspot pointer");
        return SYSTEM_NULL_PARAMETER;
    }

    if ((*wifi_hotspot)->is_initialized)
    {
        error_type_t ret = wifi_hotspot_deinit(*wifi_hotspot);
        if (ret != SYSTEM_OK)
        {
            return ret;
        }
    }

    free(*wifi_hotspot);
    *wifi_hotspot = NULL;
    ESP_LOGI(TAG, "Hotspot destroyed");
    return SYSTEM_OK;
}

error_type_t wifi_hotspot_on(wifi_hotspot_t *wifi_hotspot)
{
    if (!wifi_hotspot) {
        ESP_LOGE(TAG, "Null hotspot pointer");
        return SYSTEM_NULL_PARAMETER;
    }
    if (!wifi_hotspot->is_initialized) {
        ESP_LOGE(TAG, "Hotspot not initialized");
        return SYSTEM_INVALID_STATE;
    }
    if (wifi_hotspot->is_running) {
        ESP_LOGE(TAG, "Hotspot already running");
        return SYSTEM_INVALID_STATE;
    }

   
    wifi_auth_mode_t esp_auth_mode;
    if (map_auth_mode(wifi_hotspot->config.auth_mode, &esp_auth_mode) != ESP_OK) {
        ESP_LOGE(TAG, "Invalid auth mode");
        return SYSTEM_INVALID_PARAMETER;
    }


    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(wifi_hotspot->config.ssid),
            .channel = wifi_hotspot->config.channel,
            .max_connection = wifi_hotspot->config.max_connections,
            .authmode = esp_auth_mode,
            .pmf_cfg = {
                .required = true,
            },
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#endif
        },
    };
    strncpy((char*)wifi_config.ap.ssid, wifi_hotspot->config.ssid, 32);
    strncpy((char*)wifi_config.ap.password, wifi_hotspot->config.password, 64);
    if (strlen(wifi_hotspot->config.password) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    wifi_hotspot->is_running = true;
    ESP_LOGI(TAG, "Hotspot started. SSID:%s channel:%d", wifi_hotspot->config.ssid, wifi_hotspot->config.channel);
    return SYSTEM_OK;
}

error_type_t wifi_hotspot_off(wifi_hotspot_t *wifi_hotspot)
{
    if (!wifi_hotspot)
    {
        ESP_LOGE(TAG, "Null hotspot pointer");
        return SYSTEM_NULL_PARAMETER;
    }
    if (!wifi_hotspot->is_initialized)
    {
        ESP_LOGE(TAG, "Hotspot not initialized");
        return SYSTEM_INVALID_STATE;
    }
    if (!wifi_hotspot->is_running)
    {
        ESP_LOGE(TAG, "Hotspot not running");
        return SYSTEM_INVALID_STATE;
    }

    ESP_ERROR_CHECK(esp_wifi_stop());
    wifi_hotspot->is_running = false;
    ESP_LOGI(TAG, "Hotspot stopped");
    return SYSTEM_OK;
}



