/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "init.h"
#include <pump_control_unit.h>
#include <esp_log.h>

static const char *valid_json =
"{\n"
"  \"site_id\": \"Site123\",\n"
"  \"device_id\": \"Device456\",\n"
"  \"pump_control_units\": [\n"
"    {\n"
"      \"id\": 1,\n"
"      \"tank_monitors\": [\n"
"        { \"id\": 1, \"tank_id\": 1, \"level_sensor_id\": 1 }\n"
"      ],\n"
"      \"pump_monitors\": [\n"
"        { \"id\": 1, \"pump_id\": 1, \"current_sensor_id\": 1 }\n"
"      ],\n"
"      \"tanks\": [\n"
"        {\n"
"          \"id\": 1,\n"
"          \"capacity_litres\": 1000.0,\n"
"          \"shape\": \"RECTANGULAR\",\n"
"          \"height_cm\": 200.0,\n"
"          \"full_level_mm\": 1800,\n"
"          \"low_level_mm\": 200\n"
"        }\n"
"      ],\n"
"      \"pumps\": [\n"
"        {\n"
"          \"id\": 1,\n"
"          \"make\": \"TestPump\",\n"
"          \"power_in_hp\": 2.5,\n"
"          \"current_rating\": 10.0,\n"
"          \"min_working_current\": 0.5\n"
"        }\n"
"      ],\n"
"      \"relays\": [\n"
"        { \"id\": 1, \"pin_number\": 23 }\n"
"      ],\n"
"      \"current_sensors\": [\n"
"        {\n"
"          \"id\": 1,\n"
"          \"interface\": {\n"
"            \"type\": \"ADS1115_one\",\n"
"            \"channel\": 1\n"
"          },\n"
"          \"make\": \"ACS712\",\n"
"          \"max_current\": 20,\n"
"          \"read_mode\": \"basic\"\n"
"        },\n"
"        {\n"
"          \"id\": 2,\n"
"          \"interface\": {\n"
"            \"type\": \"internal_adc\",\n"
"            \"channel\": 0\n"
"          },\n"
"          \"make\": \"ACS712\",\n"
"          \"max_current\": 20,\n"
"          \"read_mode\": \"basic\"\n"
"        }\n"
"      ],\n"
"      \"level_sensors\": [\n"
"        {\n"
"          \"id\": 1,\n"
"          \"interface\": \"RS485\",\n"
"          \"address\": 1,\n"
"          \"protocol\": \"GA1\"\n"
"        }\n"
"      ],\n"
"      \"subscriptions\": [\n"
"        {\n"
"          \"monitor_type\": \"PUMP_MONITOR\",\n"
"          \"monitor_id\": 1,\n"
"          \"subscribers\": [\n"
"            {\n"
"              \"type\": \"RELAY\",\n"
"              \"id\": 1,\n"
"              \"response_type\": \"RELAY_RESPONSE_ONE\"\n"
"            }\n"
"          ]\n"
"        },\n"
"        {\n"
"          \"monitor_type\": \"TANK_MONITOR\",\n"
"          \"monitor_id\": 1,\n"
"          \"subscribers\": [\n"
"            {\n"
"              \"type\": \"RELAY\",\n"
"              \"id\": 1,\n"
"              \"response_type\": \"RELAY_RESPONSE_ONE\"\n"
"            }\n"
"          ]\n"
"        }\n"
"      ]\n"
"    }\n"
"  ]\n"
"}";


void app_main(void)
{
    error_type_t result;
    esp_log_level_set("*", ESP_LOG_WARN);
    result = initializa_hw_peripherals();
    if(result != SYSTEM_OK){
        ESP_LOGE("APP_MAIN", "Failed to setup peripherals with error code: %d", result);
        return;
    }
    size_t num_pump_cus = 0;
    pump_control_unit_t** pump_control_unit_manager = NULL;
    pump_control_unit_manager = (pump_control_unit_t**)malloc(sizeof(pump_control_unit_t*) * MAX_PUMP_CONTROL_UNIT_COUNT);
    if(pump_control_unit_manager == NULL){
        ESP_LOGE("APP_MAIN", "Failed to allocate memory for pump control unit manager");
        return;
    }
    result = initialize_system_from_json(valid_json, strlen(valid_json), pump_control_unit_manager, &num_pump_cus);
    if(result != SYSTEM_OK){
        ESP_LOGE("APP_MAIN", "Failed to initialize system from JSON with error code: %d", result);
        return;
    }
    while(1){
        for(size_t i = 0; i < num_pump_cus; i++){
            pump_control_unit_t* pump_control_unit = pump_control_unit_manager[i];
            // print pointer value for debugging
            ESP_LOGI("APP_MAIN", "Running pump monitor loop for pump control unit at index %d with pump control unit instance pointer: %p", i, pump_control_unit);
            result = pump_control_unit_loop_pump_monitors(pump_control_unit);
            if(result != SYSTEM_OK){
                ESP_LOGE("APP_MAIN", "Error in pump monitor loop for pump control unit at index %d with error code: %d", i, result);
            }
            // result = pump_control_unit_loop_level_monitors(pump_control_unit);
            // if(result != SYSTEM_OK){
            //     ESP_LOGE("APP_MAIN", "Error in level monitor loop for pump control unit at index %d with error code: %d", i, result);
            // }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
