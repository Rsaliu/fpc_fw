#include <setup_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "cJSON.h"
#include "esp_spiffs.h"
#include "esp_log.h"

static const char* TAG = "SETUP_CONFIG_TEST";

void setup_config_Setup(void) {
    // Set up code before each test
}

void setup_config_down(void) {
    // Clean up code after each test
}
 // dummy mounted spiff
static void mount_spiffs(void) {
    ESP_LOGI(TAG, "Mounting SPIFFS...");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    TEST_ASSERT_EQUAL(ESP_OK, esp_vfs_spiffs_register(&conf));
}
// dummy unmounted spiffs
static void unmount_spiffs(void) {
    ESP_LOGI(TAG, "Unmounting SPIFFS...");
    esp_vfs_spiffs_unregister(NULL);
}

const char* dummy_json_file() {
   
    static const char* valid_json_data =
        "{"
        "\"site_id\": \"SITE123\","
        "\"device_id\": \"DEVICE456\","
        "\"pump_control_units\": ["
        "  {"
        "    \"tank\": {"
        "      \"id\": 1,"
        "      \"capcity_in_liters\": 500.0,"
        "      \"shape\": \"Cylinder\","
        "      \"height_in_cm\": 200.0,"
        "      \"full_level_in_mm\": 1900,"
        "      \"low_level_in_mm\": 300"
        "    },"
        "    \"pump\": {"
        "      \"id\": 1,"
        "      \"make\": \"Grundfos\","
        "      \"power_in_hp\": 1.5"
        "    },"
        "    \"tank_monitor\": {"
        "      \"id\": 1,"
        "      \"level_sensor_id\": 1,"
        "      \"tank_id\": 1"
        "    },"
        "    \"pump_monitor\": {"
        "      \"id\": 1,"
        "      \"current_sensor_id\": 1,"
        "      \"pump_id\": 1"
        "    },"
        "    \"level_sensor\": {"
        "      \"id\": 1,"
        "      \"interface\": \"RS485\","
        "      \"sensor_addr\": 10,"
        "      \"level_sensor_protocol\": \"GL_A01_PROTOCOL\""
        "    },"
        "    \"relay\": {"
        "      \"id\": 1,"
        "      \"pin_number\": 25"
        "    },"
        "    \"current_sensor\": {"
        "      \"id\": 1,"
        "      \"interface\": \"I2C\","
        "      \"make\": \"ACS712\","
        "      \"max_current\": 30"
        "    }"
        "  }"
        "]"
        "}";

    return valid_json_data;
}


TEST_CASE("setup_config_test","test_tank_setup_config"){
    setup_config_Setup();

    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = tank_Setup_config(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "tank_setup_config is sucessful\n");
    
    setup_config_down();
}

TEST_CASE("setup_config_test", "test_pump_setup_config"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = pump_setup_config(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "pump_setup_config is sucessful\n");

     setup_config_down();

}

TEST_CASE("setup_config_test", "test_tank_monitor_setup_config"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = tank_monitor_setup_config(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "ptank_monitor_setup_config is sucessful\n");

     setup_config_down();

}

TEST_CASE("setup_config_test", "test_pump_monitor_setup_config"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = pump_monitor_setup_config(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "pump_ monitor_setup_config is sucessful\n");

     setup_config_down();

}

TEST_CASE("setup_config_test", "test_relay_driver_setup_config"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = relay_driver_setup_config(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "relay_setup_config is sucessful\n");

     setup_config_down();

}

TEST_CASE("setup_config_test", "test_level_sensor_setup_config"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = level_sensor_setup_config(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "level_sensor_setup_config is sucessful\n");

     setup_config_down();

}

TEST_CASE("setup_config_test", "test_current_sensor_setup_config"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = current_sensor_setup_confi(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "current_sensor_setup_config is sucessful\n");

     setup_config_down();

}

TEST_CASE("setup_config_test", "test_spi_setup_config"){
    setup_config_Setup();
    mount_spiffs();
    // dummy json file
    const char* json_content = dummy_json_file();

    // dummy file path
    const char* dummy_file_path =  "/spiffs/dummy_config.json";  
    FILE *fp = fopen(dummy_file_path, "w");
    TEST_ASSERT_NOT_NULL(fp); 
    fprintf(fp, "%s", json_content);
    fclose(fp);
    error_type_t err = spi_setup_config(dummy_file_path);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "Dummy JSON file opened and read correctly");
    
    // Cleanup the dummy file part
    remove(dummy_file_path);
    unmount_spiffs();
     setup_config_down();

}