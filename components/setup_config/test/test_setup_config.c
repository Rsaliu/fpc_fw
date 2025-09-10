#include <setup_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "cJSON.h"
#include "esp_log.h"

static const char* TAG = "SETUP_CONFIG_TEST";

void setup_config_Setup(void) {
    // Set up code before each test
}

void setup_config_teardown(void) {
    // Clean up code after each test
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
    "      \"power_in_hp\": 1.5,"
    "      \"current_rating\": 2.5"
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
    "      \"protocol\": \"GL_A01_PROTOCOL\""
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


TEST_CASE("setup_config_test","test_setup_config_tank"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = setup_config_tank(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "tank_setup_config is sucessful\n");
    cJSON_Delete(root);
    setup_config_teardown();
}

TEST_CASE("setup_config_test", "test_setup_config_pump"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = setup_config_pump(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "pump_setup_config is sucessful\n");
    cJSON_Delete(root);
     setup_config_teardown();

}

TEST_CASE("setup_config_test", "test_setup_config_tank_monitor"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = setup_config_tank_monitor(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "tank_monitor_setup_config is sucessful\n");
    cJSON_Delete(root);
     setup_config_teardown();

}

TEST_CASE("setup_config_test", "test_setup_config_pump_monitor"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = setup_config_pump_monitor(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "pump_ monitor_setup_config is sucessful\n");
    cJSON_Delete(root);
     setup_config_teardown();

}

TEST_CASE("setup_config_test", "test_setup_config_relay_driver"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = setup_config_relay_driver(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "relay_setup_config is sucessful\n");
     cJSON_Delete(root);
     setup_config_teardown();

}

TEST_CASE("setup_config_test", "test_setup_config_level_sensor"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = setup_config_level_sensor(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "level_sensor_setup_config is sucessful\n");
    cJSON_Delete(root);
    setup_config_teardown();

}

TEST_CASE("setup_config_test", "test_setup_config_current_sensor"){
    setup_config_Setup();
    const char* json_file = dummy_json_file();
    cJSON* root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    error_type_t err = setup_config_current_sensor(root);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "current_sensor_setup_config is sucessful\n");
     cJSON_Delete(root);
     setup_config_teardown();

}
