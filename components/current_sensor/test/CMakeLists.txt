cmake_minimum_required(VERSION 3.5)

idf_component_register(SRC_DIRS "."
                       INCLUDE_DIRS "."
                       REQUIRES unity
                       PRIV_REQUIRES current_sensor common cmock)
# uncomment the following line to enable hardware testing in this component's tests
# target_compile_definitions(${COMPONENT_LIB} PRIVATE HARDWARE_TESTING_ENABLED=1)