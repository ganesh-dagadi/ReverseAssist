idf_component_register(SRCS "src/UartStreamer.c" "src/serializer.c" "src/UartDriver.c"
					INCLUDE_DIRS "include"
					REQUIRES os sensor logger esp_driver_uart
					)