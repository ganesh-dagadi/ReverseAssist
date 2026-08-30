#include "DistanceSensor.h"
#include "logger.h"
#include "IDistanceSensorHal.h"
#define TAG "DistanceSensorTask"
#define DISTANCE_SENSOR_SLEEP_DUR 100 // Sample at 10Hz

Os_QueueHandle distance_data_queue;
Os_QueueHandle state_machine_commands_queue;
char m_is_streaming = 0;

void (*m_state_machine_callback)(int, int);

void distance_callback(int sensor_id, float data);
void sensor_status_callback(int sensor_id, int status);

void set_distance_sensor_queue(Os_QueueHandle queue)
{
    log_info(TAG, "Setting distance sensor queue");
    distance_data_queue = queue;
}

void set_state_machine_queue_for_distance_service(Os_QueueHandle queue)
{
    state_machine_commands_queue = queue;
}

void register_status_callback_for_distance_service(void (*state_machine_callback)(int, int))
{
    m_state_machine_callback = state_machine_callback;
}

void setup_distance_sensors()
{
    // setup
    log_info(TAG, "Setting up distance sensors");

    // read from config and setup sensors for next version
    // for now default to one sensor with id 0
    register_distance_callback(distance_callback);
    register_status_callback(sensor_status_callback);
    setup_driver();

    log_info(TAG, "Opening sensor with id 0");
    int ret = open_sensor(0);
    if (ret != 0)
    {
        log_error(TAG, "Status of open sensor with id: %d is %d", 0, ret);
        m_state_machine_callback(0, ret);
    }
}

void teardown_sensors()
{
    int ret = close_sensor(0);
    if (ret != 0)
    {
        log_error(TAG, "Status of close sensor with id: %d is %d", 0, ret);
    }
    unregister_status_callback();
    unregister_distance_callback();
}

void pause_sensor_streaming()
{
    int ret = stop_stream_distance();
    if (ret != 0)
    {
        log_error(TAG, "Status of pausing sensor stream with id: %d is %d", 0, ret);
    }
}

void distance_sensor_task_main(void *params)
{

    // loop
    while (1)
    {
        DistanceSensorCommands cmd;
        poll_queue_blocking(state_machine_commands_queue, &cmd);

        if (cmd == DISTANCE_SENSOR_CMD_START) {
            setup_distance_sensors();
        }

        if (cmd == DISTANCE_SENSOR_CMD_CLOSE) {
            teardown_sensors();
        }

        if (cmd == DISTANCE_SENSOR_CMD_PAUSE) {
            if (!m_is_streaming)
            {
                log_error(TAG, "Received pause stream when not streaming");
                continue;
            }
            log_info(TAG, "Stopping stream distance");
            if (stop_stream_distance() != 0) {
                log_error(TAG, "Failed to stop streaming for sensors");
                m_state_machine_callback(1, DISTANCE_SENSOR_ERROR);
                continue;
            }
            m_is_streaming = 0;
        }

        if (cmd == DISTANCE_SENSOR_CMD_RESUME) {
            if (m_is_streaming) {
                log_error(TAG, "Received resume stream when streaming");
                continue;
            }
            log_info(TAG, "Starting to stream the distance");
            if (stream_distance() != 0) {
                log_error(TAG, "Failed to start streaming at sensor id 1");
                m_state_machine_callback(1, DISTANCE_SENSOR_ERROR);
                continue;
            }
            m_is_streaming = 1;
        }
    }
}

void distance_callback(int sensor_id, float distance)
{
    log_debug(TAG, "Received distance data %f for sensor %d", distance, sensor_id);
    DistanceData currData;
    currData.sensor_id = sensor_id;
    currData.distance = distance;
    push_queue(distance_data_queue, &currData);
}

void sensor_status_callback(int sensor_id, int status)
{
}