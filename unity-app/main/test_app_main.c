/* Example test application for testable component.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_event.h"
static void print_banner(const char* text);

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    print_banner("Running all the registered tests");
    UNITY_BEGIN();
    unity_run_test_by_name("adc_reader_test");
    unity_run_test_by_name("acs712_current_sensor_test");
    unity_run_test_by_name("current_sensor_test");
    unity_run_test_by_name("ads1115_test");
    unity_run_test_by_name("pump_test");
    unity_run_test_by_name("level_sensor_test");
    unity_run_test_by_name("rs485_test");
    unity_run_test_by_name("tank_test");
    unity_run_test_by_name("tank_monitor_test");
    unity_run_test_by_name("pump_control_unit_test");
    unity_run_test_by_name("relay_test");
    unity_run_test_by_name("webserver_test");
    unity_run_test_by_name("wifi_hotspot_test");
    unity_run_test_by_name("config_manager_test");

    //unity_run_test_by_name("wifi_hotspot_test");
    unity_run_test_by_name("pump_monitor_test");
    unity_run_test_by_name("setup_config_test");
    UNITY_END();
}

static void print_banner(const char* text)
{
    printf("\n#### %s #####\n\n", text);
}