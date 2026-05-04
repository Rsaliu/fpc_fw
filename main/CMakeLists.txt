idf_component_register(SRCS "init.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES current_sensor common pump pump_monitor pump_control_unit factory json
                   )


                   