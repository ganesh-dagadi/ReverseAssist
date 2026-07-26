#include "IDistanceSensorHal.h"
#include "logger.h"
#define TAG "DistanceSensorHAL"

int open_sensor(int sensor_id) {
    log_info(TAG, "Opening sensor with id %d", sensor_id);
    return -1;
}

int close_sensor(int sensor_id) {
    log_info(TAG, "Closing sensor with id %d", sensor_id);
    return 0;
}

int register_time_callback(void (*distance_callback)(int,TimeData*)) {
    log_info(TAG, "Registering time callback");
    return 0;
}

int unregister_time_callback() {
    log_info(TAG, "Unregistering time callback");
    return 0;
}

int register_status_callback(void (*status_callback)(int, int)) {
    log_info(TAG, "Registering status callback");
    return 0;
}

int unregister_status_callback() {
    log_info(TAG, "Unregistering status callback");
    return 0;
}

int stream_distance(int sensor_id) {
    log_info(TAG, "Starting distance streaming");
    return 0;
}

int stop_stream_distance(int sensor_id) {
    log_info(TAG, "Stopping distance streaming");
    return 0;
}

