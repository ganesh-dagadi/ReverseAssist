#ifndef DISTANCE_SENSOR_HAL
#define DISTANCE_SENSOR_HAL

#include <stdint.h>

typedef enum DistanceSensorStatus
{
    DISTANCE_SENSOR_OK = 0,
    DISTANCE_SENSOR_ERROR,
    DISTANCE_SENSOR_INVALID,
    DISTANCE_SENSOR_SIGNAL_LOST
} DistanceSensorStatus;


int setup_driver();
int open_sensor(int sensor_id);
int close_sensor(int sensor_id);
int register_distance_callback(void (*distance_callback)(int, float));
int unregister_distance_callback();
int register_status_callback(void (*status_handler)(int,int));
int unregister_status_callback();
int stream_distance();
int stop_stream_distance();

#endif