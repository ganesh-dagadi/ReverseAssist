#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "StateMachine.h"
#include "DistanceSensor.h"
#include "esp_log.h"

static DistanceSensor* distanceSensor;
static TaskHandle_t sensorTask;
static StateMachineState currState = SLEEPING;
const char* STATE_MACHINE_TAG = "StateMachine";
const int DISTANCE_POLL_DELAY = 1000; //ms


void poll_distance_task(void* params) {
    while (1) {
        if (distanceSensor == NULL) {
            ESP_LOGE(STATE_MACHINE_TAG, "Distance Sensor is null");
        } else {
            int distance = poll_distance(distanceSensor);
            ESP_LOGI(STATE_MACHINE_TAG, "Measured distance as %d", distance);
        }
        vTaskDelay(pdMS_TO_TICKS(DISTANCE_POLL_DELAY));
    }
}
int init_state_machine() {
    // create and initialize the sensor
    DistanceSensor* mockSensor = create_distance_sensor(0);
    if (mockSensor == NULL) {
        ESP_LOGE(STATE_MACHINE_TAG, "Failed to create distance sensor");
        return -1;
    }

    if (init_distance_sensor(mockSensor) == -1) {
        ESP_LOGE(STATE_MACHINE_TAG, "Failed to initialize distance sensor");
        return -1;
    }
    distanceSensor = mockSensor;
    return 0;
    // open, configure and setup the streamer
    // create the serializer
    // plumbing of everything together
}

void start_state_machine(void* params) {
    while (1) {
        if (currState == SLEEPING) {
            currState = STARTING;
        } else if (currState == STARTING) {
            // start sensor polling
            xTaskCreate(
                poll_distance_task,
                "DISTANCE_SENSOR_TASK",
                2048,
                NULL,
                8,
                &sensorTask
            );
            currState = STARTED;
        }
        vTaskDelay(50);
    }
}