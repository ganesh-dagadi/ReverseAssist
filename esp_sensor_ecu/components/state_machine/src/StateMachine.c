#include "logger.h"
#include "os.h"
#include "StateMachine.h"
#include "DistanceSensor.h"
#define DISTANCE_SENSOR_TASK_PRIORITY 7
#define STATE_MACHINE_SLEEP_DUR 250
#define DISTANCE_SENSOR_QUEUE_LEN 20 // at 10hz, 2 seconds worth of queue capacity
#define DISTANCE_SENSOR_TASK_SPACE 2048
#define STATE_DISTANCE_CMD_QUEUE_LEN 10
#define TAG "StateMachine"

StateMachineState mState = SLEEPING;

Os_TaskHandle sensor_task;
Os_TaskHandle filter_task;
Os_TaskHandle streamer_task;
Os_TaskHandle heartbeat_task;
Os_TaskHandle property_task;

Os_QueueHandle sensor_filter_q;
Os_QueueHandle filter_streamer_q;
Os_QueueHandle state_distance_cmd_q;

void distance_sensor_status_callback(int sensor_id, int status) {
    log_info(TAG, "Received status: %d for sensor: %d", status, sensor_id);
}

int init_state_machine() {
    // read from config once config is ready

    // create the queues
    if(create_queue(DISTANCE_SENSOR_QUEUE_LEN, sizeof(DistanceData), &sensor_filter_q) != 0) {
        log_error(TAG, "Failed to create queue for distance sensor");
        return -1;
    }
    if(create_queue(DISTANCE_SENSOR_QUEUE_LEN, sizeof(DistanceData), &filter_streamer_q) != 0) {
        log_error(TAG, "Failed to create queue for distance sensor");
        return -1;
    }

    if (create_queue(STATE_DISTANCE_CMD_QUEUE_LEN, sizeof(DistanceSensorCommands), &state_distance_cmd_q) != 0){
        log_error(TAG, "Failed to create queue for state and distance command passing");
        return -1;
    }
    set_distance_sensor_queue(sensor_filter_q);
    set_state_machine_queue_for_distance_service(state_distance_cmd_q);
    register_status_callback_for_distance_service(distance_sensor_status_callback);

    DistanceSensorCommands cmd = DISTANCE_SENSOR_CMD_START;
    push_queue(state_distance_cmd_q, &cmd);

    // start the sub tasks
    if(create_task(distance_sensor_task_main, "Distance Sensor Task", DISTANCE_SENSOR_TASK_PRIORITY, DISTANCE_SENSOR_TASK_SPACE, &sensor_task) != 0) {
        log_error(TAG, "Failed to start Sensor task. Aborting");
        return -1;
    }

    return 0;

}

void start_state_machine(void* params) {
    while (1) {
        switch (mState) {
        case SLEEPING: {
            log_info(TAG, "State machine in SLEEPING State. Begin Initialization");
            if(init_state_machine() != 0) {
                log_error(TAG, "State Machine Init failed");
                mState = ERROR;
            } else {
                log_info(TAG, "State Machine init complete");
                mState = RUNNING;
            }
            break;
        }
        case ERROR: {
            log_error(TAG, "Error state");
            break;
        }
        case RUNNING: {
            log_debug(TAG, "Running state");
            break;
        }
        default:
            break;
        }
        sleep_task(STATE_MACHINE_SLEEP_DUR);
    }
}