cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "test_adc_reader.c"
                        "test_ads1115.c"
                       INCLUDE_DIRS "."
                       REQUIRES unity
                       PRIV_REQUIRES current_sensor common cmock)
# uncomment the following line to enable hardware testing in this component's tests
#target_compile_definitions(${COMPONENT_LIB} PRIVATE HARDWARE_TESTING_ENABLED=1)