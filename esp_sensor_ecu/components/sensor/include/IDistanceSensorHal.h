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

typedef struct TimeData {
    int64_t trig_time;
    int64_t recv_time;
} TimeData;

int open_sensor(int sensor_id);
int close_sensor(int sensor_id);
int register_time_callback(void (*distance_callback)(int, TimeData*));
int unregister_time_callback();
int register_status_callback(void (*status_handler)(int,int));
int unregister_status_callback();
int stream_distance(int sensor_id);
int stop_stream_distance(int sensor_id);

#endif