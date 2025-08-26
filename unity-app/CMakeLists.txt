# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/version.cmake)

# Add newly added components to one of these lines:
# 1. Add here if the component is compatible with IDF >= v4.3
set(EXTRA_COMPONENT_DIRS "../components" "../third_party/esp-protocols/components/mdns")


set(TEST_COMPONENTS "pump tank tank_monitor pump_control_unit relay_driver rs485 level_sensor crc protocol current_sensor wifi_hotspot webserver pump_monitor")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(example_test_app)