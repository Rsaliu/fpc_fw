/* Example test application for testable component.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include "unity.h"

static void print_banner(const char* text);

void app_main(void)
{

    print_banner("Running all the registered tests");
    UNITY_BEGIN();
    unity_run_test_by_name("pump_test");
    unity_run_test_by_name("level_sensor_test");
    unity_run_test_by_name("rs485_test");
    unity_run_test_by_name("relay_test");
    UNITY_END();
}

static void print_banner(const char* text)
{
    printf("\n#### %s #####\n\n", text);
}