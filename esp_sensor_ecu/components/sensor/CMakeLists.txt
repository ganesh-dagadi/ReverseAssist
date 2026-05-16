idf_component_register(SRCS "src/DistanceSensor.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_gpio esp_timer
                    )