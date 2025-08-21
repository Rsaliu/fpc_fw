#include <webserver.h>
#include <unity.h>
#include <esp_log.h>
#include <common_headers.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include <register_handler.h>
#include <login_handler.h>
#include <config_handler.h>
#include <logout_handler.h>
#include <reset_handler.h>
#include <web_server_setup.h>

const char * WEBSERVER_TEST_TAG = "WEBSERVER_TEST";

webserver_t* webserver = NULL;

webserver_config_t webserver_config = {
    .port = 80, // Port number for the web server
    .max_connections = 4, // Maximum number of concurrent connections
    .mdns_instance = "FPC-WebServer", // MDNS instance name
    .mdns_hostname = "fpc-webserver", // MDNS hostname
    .base_path = "/spiffs", // Base path for the web server
    .web_mount_point = "/web", // Web mount point for serving files
    .web_partition_label = "spiffs", // Partition label for the web server
    .config_file_path = "config.json" // Path to the configuration file
};

void webserverSetUp(void) {
    // Set up code before each test
    webserver = webserver_create(&webserver_config);
}

void webserverTearDown(void) {
    // Clean up code after each test
    if (webserver != NULL) {
        webserver_destroy(&webserver);
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(WEBSERVER_TEST_TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(WEBSERVER_TEST_TAG, "station "MACSTR" leave, AID=%d, reason=%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    //ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32-WebServer",
            .ssid_len = strlen("ESP32-WebServer"),
            .channel = 1,
            .password = "password123",
            .max_connection = 5,
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
            .authmode = WIFI_AUTH_WPA2_PSK,
#endif
            .pmf_cfg = {
                    .required = true,
            },
#ifdef CONFIG_ESP_WIFI_BSS_MAX_IDLE_SUPPORT
            .bss_max_idle_cfg = {
                .period = WIFI_AP_DEFAULT_MAX_IDLE_PERIOD,
                .protected_keep_alive = 1,
            },
#endif
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

}


// dummy test case to start WiFi

TEST_CASE("webserver_test", "dummy_test") {
    // This is a dummy test case to startup WiFi AP
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_softap();
    ESP_LOGI(WEBSERVER_TEST_TAG, "Web server test initialized");
}

TEST_CASE("webserver_test", "test_webserver_create") {
    webserverSetUp();
    TEST_ASSERT_NOT_NULL(webserver);
    webserverTearDown();
}

TEST_CASE("webserver_test", "test_webserver_init") {
    webserverSetUp();
    error_type_t result = webserver_init(webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    webserverTearDown();
}

TEST_CASE("webserver_test", "test_webserver_start") {
    webserverSetUp();
    error_type_t result = webserver_init(webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = webserver_start(webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    webserverTearDown();
}

TEST_CASE("webserver_test", "test_webserver_destroy") {
    webserverSetUp();
    error_type_t result = webserver_init(webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = webserver_destroy(&webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(webserver); // Web server should be NULL after destruction
}

TEST_CASE("webserver_test", "test_webserver_deinit") {
    webserverSetUp();
    error_type_t result = webserver_init(webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = webserver_deinit(webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    webserverTearDown();
}

TEST_CASE("webserver_test", "test_webserver_add_route") {
    webserverSetUp();
    error_type_t result = webserver_init(webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    //start the web server before adding routes
    result = webserver_start(webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    httpd_uri_t uri = {
        .uri = "/test",
        .method = HTTP_GET,
        .handler = NULL, // Set a valid handler function here
        .user_ctx = NULL
    };

    result = webserver_add_route(webserver, &uri);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    webserverTearDown();
}

TEST_CASE("webserver_test", "test_webserver_remove_route") {
    webserverSetUp();
    error_type_t result = webserver_init(webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    //start the web server before adding routes
    result = webserver_start(webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    httpd_uri_t uri = {
        .uri = "/test",
        .method = HTTP_GET,
        .handler = NULL, // Set a valid handler function here
        .user_ctx = NULL
    };

    result = webserver_add_route(webserver, &uri);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result = webserver_remove_route(webserver, uri.uri, uri.method);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    webserverTearDown();
}
// dummy request handler for testing
esp_err_t dummy_handler(httpd_req_t *req) {
    char *rest_context = (char *)req->user_ctx;
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Hello World! : %s", rest_context);
    ESP_LOGI(WEBSERVER_TEST_TAG, "Handling request for URI: %s with context: %s", req->uri, rest_context);
    httpd_resp_send(req, buffer, strlen(buffer));
    return ESP_OK;
}

TEST_CASE("webserver_test", "test_webserver_add_dummy_route") {
    webserverSetUp();
    error_type_t result = webserver_init(webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    //start the web server before adding routes
    result = webserver_start(webserver);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    rest_server_context_t *context_ptr = NULL;
    result = webserver_get_context(webserver, &context_ptr);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = setup_web_handlers(webserver,context_ptr);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    //webserverTearDown();
}

//dummy test to switch off wifi-ap hotspot
TEST_CASE("webserver_test", "test_webserver_stop_wifi_ap") {
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_LOGI(WEBSERVER_TEST_TAG, "WiFi AP stopped");
}
