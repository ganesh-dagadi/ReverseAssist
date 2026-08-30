#ifndef DISTANCE_SENSOR
#define DISTANCE_SENSOR

#include "os.h"
typedef struct DistanceSensor DistanceSensor;

typedef struct {
    uint8_t sensor_id;
    float distance;
} DistanceData;

typedef enum DistanceSensorCommands {
    DISTANCE_SENSOR_CMD_START = 0,
    DISTANCE_SENSOR_CMD_CLOSE,
    DISTANCE_SENSOR_CMD_PAUSE,
    DISTANCE_SENSOR_CMD_RESUME
}DistanceSensorCommands;

void distance_sensor_task_main(void* params);
void register_status_callback_for_distance_service(void (*state_machine_callback)(int, int)); // sensor_id, status if sensor_id is 0, service error
void set_distance_sensor_queue(Os_QueueHandle handle);
void set_state_machine_queue_for_distance_service(Os_QueueHandle handle);

#endif