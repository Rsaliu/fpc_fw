#include "unity.h"
#include "validate_json.h"
#include <esp_err.h>
#include <string.h>
#include <stdlib.h>

static const char *valid_json =
"{\n"
"    \"Site Id\": \"Site123\",\n"
"    \"Device Id\": \"Device456\",\n"
"    \"Pump Control Units\": [\n"
"        {\n"
"            \"Id\": 1,\n"
"            \"tank_monitors\": [\n"
"                {\"Id\": 1, \"tank Id\": 1, \"level sensor Id\": 1}\n"
"            ],\n"
"            \"pump_monitors\": [\n"
"                {\"Id\": 1, \"Pump Id\": 1, \"Current Sensor Id\": 1}\n"
"            ],\n"
"            \"tanks\": [\n"
"                {\"Id\": 1, \"capacity In Litres\": 1000.0, \"shape\": \"RECTANGLE\", \"height In cm\": 200.0, \"full level mm\": 1800, \"low level mm\": 200}\n"
"            ],\n"
"            \"pumps\": [\n"
"                {\"Id\": 1, \"make\": \"TestPump\", \"power_in_hp\": 2.5, \"current_rating\": 10.0}\n"
"            ],\n"
"            \"Relays\": [\n"
"                {\"Id\": 1, \"Relay_pin_number\": 23}\n"
"            ],\n"
"            \"current_sensor\": [\n"
"                {\"Id\": 1, \"interface\": \"I2C\", \"make\": \"ACS712\", \"max_current\": 20}\n"
"            ],\n"
"            \"Level_sensor\": {\"Id\": 1, \"interface\": \"RS485\", \"sensor_add\": 1, \"protocol\": \"GA1\"}\n"
"        }\n"
"    ],\n"
"    \"mappings\": [\n"
"        {\"tank_monitor_ids\": [1], \"pump_monitor_ids\": [1], \"relay_ids\": [1]}\n"
"    ]\n"
"}";


static const char *invalid_missing_field =
"{\"Device Id\": \"Device456\", \"Pump Control Units\": []}";

static const char *invalid_wrong_type =
"{\"Site Id\": \"Site123\", \"Device Id\": \"Device456\", "
"\"Pump Control Units\": [{\"Id\": \"string_instead_of_int\", \"tank_monitors\": []}]}";

static const char *invalid_malformed =
"{\"Site Id\": \"Site123\" "; 

static const char *invalid_invalid_enum =
"{\n"
"    \"Site Id\": \"Site123\",\n"
"    \"Device Id\": \"Device456\",\n"
"    \"Pump Control Units\": [\n"
"        {\n"
"            \"Id\": 1,\n"
"            \"tank_monitors\": [],\n"
"            \"pump_monitors\": [],\n"
"            \"tanks\": [\n"
"                {\"Id\": 1, \"capacity In Litres\": 1000.0, \"shape\": \"INVALID_SHAPE\", \"height In cm\": 200.0, \"full level mm\": 1800, \"low level mm\": 200}\n"
"            ],\n"
"            \"pumps\": [],\n"
"            \"Relays\": [],\n"
"            \"current_sensor\": [],\n"
"            \"Level_sensor\": {\"Id\": 1, \"interface\": \"RS485\", \"sensor_add\": 1, \"protocol\": \"GA1\"}\n"
"        }\n"
"    ],\n"
"    \"mappings\": []\n"
"}";


static void run_validate_test(const char *json_str, esp_err_t expected)
{
    esp_err_t result = validate_json(json_str, strlen(json_str));
    TEST_ASSERT_EQUAL(expected, result);
}


TEST_CASE("validate_json_test", "test_validate_json_valid_full")
{
    run_validate_test(valid_json, ESP_OK);
}

TEST_CASE( "validate_json_test", "test_validate_json_missing_field")
{
    run_validate_test(invalid_missing_field, ESP_FAIL);
}

TEST_CASE("validate_json_test", "test_validate_json_wrong_type")
{
    run_validate_test(invalid_wrong_type, ESP_FAIL);
}

TEST_CASE("validate_json_test","test_validate_json_malformed")
{
    run_validate_test(invalid_malformed, ESP_FAIL);
}

TEST_CASE("validate_json_test", "test_validate_json_invalid_enum")
{
    run_validate_test(invalid_invalid_enum, ESP_FAIL);
}

TEST_CASE("validate_json_test", "test_validate_json_null_input")
{
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, validate_json(NULL, 10));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, validate_json(valid_json, 0));
}
