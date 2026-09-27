idf_component_register(SRCS "src/DistanceSensor.c" "src/DistanceSensorHal.c"
                    INCLUDE_DIRS "include"
                    REQUIRES os logger esp_driver_gpio esp_timer
                    )