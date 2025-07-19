cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/acs712_current_sensor.c" 
                    "src/adc_reader.c"
                    "src/current_sensor.c"
                    "src/current_sensor_context.c"
                    "src/ads1115.c"
                       INCLUDE_DIRS "."
                       REQUIRES common driver esp_adc
                       )



