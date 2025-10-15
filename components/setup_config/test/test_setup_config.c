#include <setup_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "esp_log.h"
#include "cJSON.h"

static const char *TAG = "SETUP_CONFIG_TEST";
pump_control_unit_t unit;

void setup_config_Setup(void)
{
    // Set up code before each test
}

void setup_config_teardown(void)
{
    // Clean up code after each test
}

const char *dummy_json_file()
{
    static const char *valid_json_data =
        "{"
        "\"site_id\": \"SITE123\","
        "\"device_id\": \"DEVICE456\","
        "\"pump_control_units\": ["
        "  {"
        "    \"id\": 1,"
        "    \"tank\": {"
        "      \"id\": 1,"
        "      \"capacity_in_liters\": 500.0,"
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
        "  },"
        "  {"
        "    \"id\": 2,"
        "    \"tank\": {"
        "      \"id\": 2,"
        "      \"capacity_in_liters\": 800.0,"
        "      \"shape\": \"Square\","
        "      \"height_in_cm\": 180.0,"
        "      \"full_level_in_mm\": 1700,"
        "      \"low_level_in_mm\": 400"
        "    },"
        "    \"pump\": {"
        "      \"id\": 2,"
        "      \"make\": \"Pedrollo\","
        "      \"power_in_hp\": 2.0,"
        "      \"current_rating\": 3.0"
        "    },"
        "    \"tank_monitor\": {"
        "      \"id\": 2,"
        "      \"level_sensor_id\": 2,"
        "      \"tank_id\": 2"
        "    },"
        "    \"pump_monitor\": {"
        "      \"id\": 2,"
        "      \"current_sensor_id\": 2,"
        "      \"pump_id\": 2"
        "    },"
        "    \"level_sensor\": {"
        "      \"id\": 2,"
        "      \"interface\": \"RS485\","
        "      \"sensor_addr\": 20,"
        "      \"protocol\": \"GL_A01_PROTOCOL\""
        "    },"
        "    \"relay\": {"
        "      \"id\": 2,"
        "      \"pin_number\": 26"
        "    },"
        "    \"current_sensor\": {"
        "      \"id\": 2,"
        "      \"interface\": \"I2C\","
        "      \"make\": \"ACS758\","
        "      \"max_current\": 50"
        "    }"
        "  },"
        "  {"
        "    \"id\": 3,"
        "    \"tank\": {"
        "      \"id\": 3,"
        "      \"capacity_in_liters\": 800.0,"
        "      \"shape\": \"Square\","
        "      \"height_in_cm\": 180.0,"
        "      \"full_level_in_mm\": 1700,"
        "      \"low_level_in_mm\": 400"
        "    },"
        "    \"pump\": {"
        "      \"id\": 3,"
        "      \"make\": \"Pedrollo\","
        "      \"power_in_hp\": 2.0,"
        "      \"current_rating\": 3.0"
        "    },"
        "    \"tank_monitor\": {"
        "      \"id\": 3,"
        "      \"level_sensor_id\": 2,"
        "      \"tank_id\": 2"
        "    },"
        "    \"pump_monitor\": {"
        "      \"id\": 3,"
        "      \"current_sensor_id\": 2,"
        "      \"pump_id\": 2"
        "    },"
        "    \"level_sensor\": {"
        "      \"id\": 3,"
        "      \"interface\": \"RS485\","
        "      \"sensor_addr\": 20,"
        "      \"protocol\": \"GL_A01_PROTOCOL\""
        "    },"
        "    \"relay\": {"
        "      \"id\": 3,"
        "      \"pin_number\": 26"
        "    },"
        "    \"current_sensor\": {"
        "      \"id\": 3,"
        "      \"interface\": \"I2C\","
        "      \"make\": \"ACS758\","
        "      \"max_current\": 50"
        "    }"
        "  }"
        "]"
        "}";
    return valid_json_data;
}

TEST_CASE("setup_config_test", "test_desrilized_pump_control_unit")
{
    setup_config_Setup();
    const char *json_file = dummy_json_file();
    cJSON *root = cJSON_Parse(json_file);
    TEST_ASSERT_NOT_EQUAL(NULL, root);
    cJSON *pump_control_units = cJSON_GetObjectItem(root, "pump_control_units");
    int unit_count = cJSON_GetArraySize(pump_control_units);
    for (int i = 0; i < unit_count; i++)
    {
        cJSON *unit_json = cJSON_GetArrayItem(pump_control_units, i);
        char *unit_str = cJSON_PrintUnformatted(unit_json);

        unit = deserilalized_pump_control_unit(unit_str);

        ESP_LOGI(TAG, "Successfully deserialized pump control unit with ID: %d\n.", unit.unit_id);

        free(unit_str);
    }

    cJSON_Delete(root);
    setup_config_teardown();
}

TEST_CASE("setup_config_test", "test_setup_config_tank")
{
    setup_config_Setup();
    error_type_t err = setup_config_tank(&unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "tank_setup_config is sucessful\n");
    setup_config_teardown();
}

TEST_CASE("setup_config_test", "test_setup_config_pump")
{
    setup_config_Setup();
    error_type_t err = setup_config_pump(&unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "pump_setup_config is sucessful\n");

    setup_config_teardown();
}

TEST_CASE("setup_config_test", "test_setup_config_tank_monitor")
{
    setup_config_Setup();
    error_type_t err = setup_config_tank_monitor(&unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "tank_monitor_setup_config is sucessful\n");

    setup_config_teardown();
}

TEST_CASE("setup_config_test", "test_setup_config_pump_monitor")
{
    setup_config_Setup();

    error_type_t err = setup_config_pump_monitor(&unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "pump_ monitor_setup_config is sucessful\n");
    setup_config_teardown();
}

TEST_CASE("setup_config_test", "test_setup_config_relay_driver")
{
    setup_config_Setup();

    error_type_t err = setup_config_relay(&unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "relay_setup_config is sucessful\n");

    setup_config_teardown();
}

TEST_CASE("setup_config_test", "test_setup_config_level_sensor")
{
    setup_config_Setup();

    error_type_t err = setup_config_level_sensor(&unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);
    ESP_LOGI(TAG, "level_sensor_setup_config is sucessful\n");
    setup_config_teardown();
}

TEST_CASE("setup_config_test", "test_setup_config_current_sensor")
{
    setup_config_Setup();

    error_type_t err = setup_config_current_sensor(&unit);
    TEST_ASSERT_EQUAL(SYSTEM_OK, err);

    setup_config_teardown();
}
