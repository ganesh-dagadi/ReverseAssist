#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "StateMachine.h"
#include "DistanceSensor.h"
#include "Filter.h"
#include "esp_log.h"

static DistanceSensor* distanceSensor;
static TaskHandle_t sensorTask;
static StateMachineState currState = SLEEPING;
static TaskHandle_t poll_sensor_task;

const char* STATE_MACHINE_TAG = "StateMachine";
const int DISTANCE_POLL_DELAY = 1000; //ms

void poll_distance_task(void* params) {
    while (1) {
        if (distanceSensor == NULL) {
            ESP_LOGE(STATE_MACHINE_TAG, "Distance Sensor is null");
        } else {
            poll_distance(distanceSensor);
        }
        vTaskDelay(pdMS_TO_TICKS(DISTANCE_POLL_DELAY));
    }
}

int create_distance_sensors() {
    DistanceSensor* lDistanceSensor = create_distance_sensor(0);
    if (lDistanceSensor == NULL) {
        ESP_LOGE(STATE_MACHINE_TAG, "Failed to create distance sensor");
        return -1;
    }

    if (init_distance_sensor(lDistanceSensor) == -1) {
        ESP_LOGE(STATE_MACHINE_TAG, "Failed to initialize distance sensor");
        close_sensor(lDistanceSensor);
        return -1;
    }

    distanceSensor = lDistanceSensor;
    return 0;
}

int init_state_machine() {
    // create and initialize the sensor
    currState = STARTING;
    if (create_distance_sensors() != 0) {
        ESP_LOGE(STATE_MACHINE_TAG, "Creating Distance sensor failed");
        currState = ERROR;
        return -1;
    }  

    // task to poll the sensor
    xTaskCreate(
        poll_distance_task,
        "DISTANCE_SENSOR_TASK",
        2048,
        NULL,
        7,
        &poll_sensor_task
    );
    
    // open, configure and setup the filter
    set_filter_strategy(MOVING_AVG_FILTER);
    start_distance_filter();
    // create the serializer
    // plumbing of everything together
    currState = RUNNING;
    return 0;
}

void start_state_machine(void* params) {
    while (1) {
        if (currState == SLEEPING) {
            init_state_machine();
        } else if (currState == RUNNING) {
            // NOTHING TO DO
            
        } else if (currState == ERROR) {
            ESP_LOGE(STATE_MACHINE_TAG, "Error state in state machine");
        }
        vTaskDelay(50);
    }
}