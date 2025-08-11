#include "wifi_hotspot.h"
#include <unity.h>
#include <common_headers.h>
#include <string.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "TEST_WIFI_HOTSPOT";

wifi_hotspot_t *hotspot = NULL;

void wifiHotspotSetUp(void)
{
    wifi_hotspot_config_t config = {
        .ssid = "TestHotspot",
        .password = "securepass123",
        .channel = 6,
        .max_connections = 4,
        .auth_mode = WIFI_HOTSPOT_AUTH_WPA2_PSK
    };
    hotspot = wifi_hotspot_create(config);
    if (hotspot == NULL) {
        ESP_LOGE(TAG, "wifi_hotspot_create failed");
    }
    
}

void wifiHotspotTearDown(void)
{
    if (hotspot != NULL)
    {
        wifi_hotspot_destroy(&hotspot);
    }
}

TEST_CASE("wifi_hotspot_test", "test_wifi_hotspot_create")
{
    wifiHotspotSetUp();
    TEST_ASSERT_NOT_NULL(hotspot);
    wifiHotspotTearDown();
}

TEST_CASE("wifi_hotspot_test", "test_wifi_hotspot_create_invalid_config")
{
    wifi_hotspot_config_t config = {
        .ssid = "", 
        .password = "securepass123",
        .channel = 1,
        .max_connections = 4,
        .auth_mode = WIFI_HOTSPOT_AUTH_WPA2_PSK
    };
    hotspot = wifi_hotspot_create(config);
    TEST_ASSERT_NULL(hotspot);

    strcpy(config.ssid, "ValidSSID");
    strcpy(config.password, "123");
    TEST_ASSERT_NULL(wifi_hotspot_create(config));

    strcpy(config.password, "securepass123");
    config.channel = 20;
    TEST_ASSERT_NULL(wifi_hotspot_create(config));

    config.channel = 6;
    config.max_connections = 20;
    TEST_ASSERT_NULL(wifi_hotspot_create(config));
}

TEST_CASE("wifi_hotspot_test", "test_wifi_hotspot_init")
{
    wifiHotspotSetUp();
    error_type_t result = wifi_hotspot_init(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    ESP_LOGI(TAG, "Hotspot initialized");
    wifiHotspotTearDown();
}

TEST_CASE("wifi_hotspot_test", "test_wifi_hotspot_init_null")
{
    error_type_t result = wifi_hotspot_init(NULL);
    TEST_ASSERT_EQUAL(SYSTEM_NULL_PARAMETER, result);
}

TEST_CASE("wifi_hotspot_test", "test_wifi_hotspot_deinit")
{
    wifiHotspotSetUp();
    error_type_t result = wifi_hotspot_init(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = wifi_hotspot_deinit(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    ESP_LOGI(TAG, "Hotspot deinitialized");
    wifiHotspotTearDown();
}

TEST_CASE("wifi_hotspot_test", "test_wifi_hotspot_deinit_null")
{
    error_type_t result = wifi_hotspot_deinit(NULL);
    TEST_ASSERT_EQUAL(SYSTEM_NULL_PARAMETER, result);
}

TEST_CASE("wifi_hotspot_test", "test_wifi_hotspot_destroy")
{
    wifiHotspotSetUp();
    error_type_t result = wifi_hotspot_init(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = wifi_hotspot_destroy(&hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(hotspot);
    ESP_LOGI(TAG, "Hotspot destroyed");
    wifiHotspotTearDown();
}

TEST_CASE("wifi_hotspot_test", "test_wifi_hotspot_destroy_null")
{
    wifi_hotspot_t *null_hotspot = NULL;
    error_type_t result = wifi_hotspot_destroy(&null_hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_NULL_PARAMETER, result);
}

TEST_CASE("wifi_hotspot_test", "test_wifi_hotspot_on")
{
    wifiHotspotSetUp();
    TEST_ASSERT_NOT_NULL(hotspot);
   
    error_type_t result = wifi_hotspot_init(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    ESP_LOGI(TAG, "Hotspot initialized");

    result = wifi_hotspot_on(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
   
    //vTaskDelay(60000 / portTICK_PERIOD_MS); 

    wifiHotspotTearDown();
    ESP_LOGI(TAG, "Test test_wifi_hotspot_on completed");
}

TEST_CASE("wifi_hotspot_test", "test_wifi_hotspot_on_invalid_state")
{
    wifiHotspotSetUp();
    error_type_t result = wifi_hotspot_on(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_STATE, result);

    result = wifi_hotspot_init(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = wifi_hotspot_on(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = wifi_hotspot_on(hotspot); 
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_STATE, result);
    wifiHotspotTearDown();
}

TEST_CASE("wifi_hotspot_test", "test_wifi_hotspot_off")
{
    wifiHotspotSetUp();
    error_type_t result = wifi_hotspot_init(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = wifi_hotspot_on(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = wifi_hotspot_off(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    wifiHotspotTearDown();
}

TEST_CASE("wifi_hotspot_test", "test_wifi_hotspot_off_invalid_state")
{
    wifiHotspotSetUp();
    error_type_t result = wifi_hotspot_off(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_STATE, result);

    result = wifi_hotspot_init(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = wifi_hotspot_off(hotspot);
    TEST_ASSERT_EQUAL(SYSTEM_INVALID_STATE, result);
    wifiHotspotTearDown();
}

