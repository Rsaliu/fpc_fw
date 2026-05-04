#ifndef __INIT_H__
#define __INIT_H__
#include <common_headers.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "adc_reader.h"
#include "current_sensor.h"
#include "ads1115.h"
#include "acs712_current_sensor.h"
#include <esp_log.h>
#include <pump.h>
#include <pump_monitor.h>
#include <current_analytics.h>
#include "esp_random.h"
#include <pump_control_unit.h>
#include <validate_json.h>
#include <setup_config.h>
#include <factory.h>
#include <emhashmap.h>
#include <rs485.h>
#include <globals.h>
#define MAX_PUMP_CONTROL_UNIT_COUNT 5
error_type_t initializa_hw_peripherals();
error_type_t initialize_system_from_json(const char* json_str, size_t size, pump_control_unit_t** pump_control_unit_manager, size_t* num_pump_control_units);

#endif