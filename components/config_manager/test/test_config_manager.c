#include "unity.h"
#include "config_manager.h"
#include <esp_err.h>
#include <cJSON.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>

static const char *TAG = "TEST_CONFIG_MANAGER";

extern void config_manager_set_reader(esp_err_t (*func)(const char *, char **, size_t *));

// Sample JSON strings
static const char *valid_json = 
"{\n"
"    \"Site Id\": \"Site123\",\n"
"    \"Device Id\": \"Device456\",\n"
"    \"Pump Control Units\": [\n"
"        {\n"
"            \"Id\": 1,\n"
"            \"tank_monitors\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"tank Id\": 1,\n"
"                    \"level sensor Id\": 1\n"
"                }\n"
"            ],\n"
"            \"pump_monitors\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"Pump Id\": 1,\n"
"                    \"Current Sensor Id\": 1\n"
"                }\n"
"            ],\n"
"            \"tanks\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"capacity In Litres\": 1000.0,\n"
"                    \"shape\": \"RECTANGLE\",\n"
"                    \"height In cm\": 200.0,\n"
"                    \"full level mm\": 1800,\n"
"                    \"low level mm\": 200\n"
"                }\n"
"            ],\n"
"            \"pumps\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"make\": \"TestPump\",\n"
"                    \"power_in_hp\": 2.5,\n"
"                    \"current_rating\": 10.0\n"
"                }\n"
"            ],\n"
"            \"Relays\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"Relay_pin_number\": 23\n"
"                }\n"
"            ],\n"
"            \"current_sensor\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"interface\": \"I2C\",\n"
"                    \"make\": \"ACS712\",\n"
"                    \"max_current\": 20\n"
"                }\n"
"            ],\n"
"            \"Level_sensor\": {\n"
"                \"Id\": 1,\n"
"                \"interface\": \"RS485\",\n"
"                \"sensor_add\": 1,\n"
"                \"protocol\": \"GA1\"\n"
"            }\n"
"        }\n"
"    ],\n"
"    \"mappings\": [\n"
"        {\n"
"            \"tank_monitor_ids\": [1],\n"
"            \"pump_monitor_ids\": [1],\n"
"            \"relay_ids\": [1]\n"
"        }\n"
"    ]\n"
"}";

static const char *invalid_json_missing_field = 
"{\n"
"    \"Device Id\": \"Device456\",\n"
"    \"Pump Control Units\": []\n"
"}";

static const char *invalid_json_wrong_type = 
"{\n"
"    \"Site Id\": \"Site123\",\n"
"    \"Device Id\": \"Device456\",\n"
"    \"Pump Control Units\": [\n"
"        {\n"
"            \"Id\": \"string_instead_of_int\",\n"
"            \"tank_monitors\": []\n"
"        }\n"
"    ]\n"
"}";

static const char *invalid_json_malformed = 
"{ \"Site Id\": \"Site123\" "; // Missing closing brace

static const char *invalid_json_invalid_enum = 
"{\n"
"    \"Site Id\": \"Site123\",\n"
"    \"Device Id\": \"Device456\",\n"
"    \"Pump Control Units\": [\n"
"        {\n"
"            \"Id\": 1,\n"
"            \"tank_monitors\": [],\n"
"            \"pump_monitors\": [],\n"
"            \"tanks\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"capacity In Litres\": 1000.0,\n"
"                    \"shape\": \"INVALID_SHAPE\",\n"
"                    \"height In cm\": 200.0,\n"
"                    \"full level mm\": 1800,\n"
"                    \"low level mm\": 200\n"
"                }\n"
"            ],\n"
"            \"pumps\": [],\n"
"            \"Relays\": [],\n"
"            \"current_sensor\": [],\n"
"            \"Level_sensor\": {\n"
"                \"Id\": 1,\n"
"                \"interface\": \"RS485\",\n"
"                \"sensor_add\": 1,\n"
"                \"protocol\": \"GA1\"\n"
"            }\n"
"        }\n"
"    ],\n"
"    \"mappings\": []\n"
"}";

static const char *multi_json = 
"{\n"
"    \"Site Id\": \"Site123\",\n"
"    \"Device Id\": \"Device456\",\n"
"    \"Pump Control Units\": [\n"
"        {\n"
"            \"Id\": 1,\n"
"            \"tank_monitors\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"tank Id\": 1,\n"
"                    \"level sensor Id\": 1\n"
"                },\n"
"                {\n"
"                    \"Id\": 2,\n"
"                    \"tank Id\": 2,\n"
"                    \"level sensor Id\": 1\n"
"                }\n"
"            ],\n"
"            \"pump_monitors\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"Pump Id\": 1,\n"
"                    \"Current Sensor Id\": 1\n"
"                },\n"
"                {\n"
"                    \"Id\": 2,\n"
"                    \"Pump Id\": 2,\n"
"                    \"Current Sensor Id\": 2\n"
"                },\n"
"                {\n"
"                    \"Id\": 3,\n"
"                    \"Pump Id\": 3,\n"
"                    \"Current Sensor Id\": 3\n"
"                }\n"
"            ],\n"
"            \"tanks\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"capacity In Litres\": 1000.0,\n"
"                    \"shape\": \"RECTANGLE\",\n"
"                    \"height In cm\": 200.0,\n"
"                    \"full level mm\": 1800,\n"
"                    \"low level mm\": 200\n"
"                },\n"
"                {\n"
"                    \"Id\": 2,\n"
"                    \"capacity In Litres\": 1500.0,\n"
"                    \"shape\": \"CYLINDER\",\n"
"                    \"height In cm\": 250.0,\n"
"                    \"full level mm\": 2200,\n"
"                    \"low level mm\": 300\n"
"                }\n"
"            ],\n"
"            \"pumps\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"make\": \"TestPump1\",\n"
"                    \"power_in_hp\": 2.5,\n"
"                    \"current_rating\": 10.0\n"
"                },\n"
"                {\n"
"                    \"Id\": 2,\n"
"                    \"make\": \"TestPump2\",\n"
"                    \"power_in_hp\": 3.0,\n"
"                    \"current_rating\": 12.0\n"
"                },\n"
"                {\n"
"                    \"Id\": 3,\n"
"                    \"make\": \"TestPump3\",\n"
"                    \"power_in_hp\": 4.0,\n"
"                    \"current_rating\": 15.0\n"
"                }\n"
"            ],\n"
"            \"Relays\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"Relay_pin_number\": 23\n"
"                }\n"
"            ],\n"
"            \"current_sensor\": [\n"
"                {\n"
"                    \"Id\": 1,\n"
"                    \"interface\": \"I2C\",\n"
"                    \"make\": \"ACS712\",\n"
"                    \"max_current\": 20\n"
"                }\n"
"            ],\n"
"            \"Level_sensor\": {\n"
"                \"Id\": 1,\n"
"                \"interface\": \"RS485\",\n"
"                \"sensor_add\": 1,\n"
"                \"protocol\": \"GA1\"\n"
"            }\n"
"        }\n"
"    ],\n"
"    \"mappings\": [\n"
"        {\n"
"            \"tank_monitor_ids\": [1, 2],\n"
"            \"pump_monitor_ids\": [1, 2, 3],\n"
"            \"relay_ids\": [1]\n"
"        }\n"
"    ]\n"
"}";

// Global to control mock JSON
static const char *mock_json = NULL;


//Mock reader
static esp_err_t mock_read_config_json(const char *path, char **buffer, size_t *size) {
    if (!mock_json) {
        ESP_LOGE(TAG, "Mock JSON not set");
        return ESP_FAIL;
    }
    *size = strlen(mock_json);
    *buffer = (char *)malloc(*size + 1);
    if (*buffer == NULL) {
        ESP_LOGE(TAG, "Mock allocation failed");
        return ESP_ERR_NO_MEM;
    }
    strcpy(*buffer, mock_json);
    return ESP_OK;
}


void configManagerSetup(void) {
    mock_json = valid_json; // Default to valid JSON
    config_manager_set_reader(mock_read_config_json); // Inject mock
}

void configManagerTearDown(void) {
    mock_json = NULL; 
    config_manager_set_reader(NULL); // Reset to default
}

// Tests
TEST_CASE("config_manager_test", "test_read_config_json_success")
{
    configManagerSetup();
    char *buffer = NULL;
    size_t size = 0;

    esp_err_t ret = mock_read_config_json("/spiffs/test.json", &buffer, &size);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_GREATER_THAN(0, size);
    TEST_ASSERT_TRUE(strstr(buffer, "Site Id") != NULL);
    free(buffer);
    configManagerTearDown();
}

TEST_CASE("config_manager_test", "test_read_config_json_null_args")
{
    configManagerSetup();
    char *buffer = NULL;
    size_t size = 0;

    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, read_config_json(NULL, &buffer, &size));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, read_config_json("/spiffs/test.json", NULL, &size));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, read_config_json("/spiffs/test.json", &buffer, NULL));
    configManagerTearDown();
}

TEST_CASE("config_manager_test", "test_validate_config_json_valid_structure")
{
    configManagerSetup();
    mock_json = valid_json;
    TEST_ASSERT_EQUAL(ESP_OK, validate_config_json("/spiffs/valid.json"));
    configManagerTearDown();
}

TEST_CASE("config_manager_test", "test_validate_config_json_invalid_syntax")
{
    configManagerSetup();
    mock_json = invalid_json_malformed;
    TEST_ASSERT_NOT_EQUAL(ESP_OK, validate_config_json("/spiffs/invalid_syntax.json"));
    configManagerTearDown();
}

TEST_CASE("config_manager_test", "test_validate_config_json_missing_field")
{
    configManagerSetup();
    mock_json = invalid_json_missing_field;
    TEST_ASSERT_NOT_EQUAL(ESP_OK, validate_config_json("/spiffs/missing_field.json"));
    configManagerTearDown();
}

TEST_CASE("config_manager_test", "test_validate_config_json_wrong_type")
{
    configManagerSetup();
    mock_json = invalid_json_wrong_type;
    TEST_ASSERT_NOT_EQUAL(ESP_OK, validate_config_json("/spiffs/wrong_type.json"));
    configManagerTearDown();
}

TEST_CASE("config_manager_test", "test_validate_config_json_invalid_enum")
{
    configManagerSetup();
    mock_json = invalid_json_invalid_enum;
    TEST_ASSERT_NOT_EQUAL(ESP_OK, validate_config_json("/spiffs/invalid_enum.json"));
    configManagerTearDown();
}

TEST_CASE("config_manager_test", "test_validate_config_json_variable_quantities")
{
    configManagerSetup();
    mock_json = multi_json;
    TEST_ASSERT_EQUAL(ESP_OK, validate_config_json("/spiffs/multi.json"));
    configManagerTearDown();
}

TEST_CASE("config_manager_test", "test_validate_config_json_null_path")
{
    configManagerSetup();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, validate_config_json(NULL));
    configManagerTearDown();
}
