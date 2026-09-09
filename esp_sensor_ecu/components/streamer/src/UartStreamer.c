#include "Streamer.h"
#include "esp_log.h"

#define TAG "Uart Streamer"


static Os_TaskHandle streamer_task_handle = NULL;
static Os_QueueHandle filtered_distance_queue = NULL;

void streamer_task_runnable(void*);

int start_streamer(Os_QueueHandle input_queue) {
    if (input_queue == NULL || streamer_task_handle != NULL) {
        return -1;
    }
    filtered_distance_queue = input_queue;
    return create_task(streamer_task_runnable, "UART_STREAMER_TASK", 5, 2048,
                       &streamer_task_handle);
}

void streamer_task_runnable(void*) {
    while (1) {
        DistanceData dis_data;
        if (poll_queue_blocking(filtered_distance_queue, &dis_data) == 0) {
            ESP_LOGI(TAG, "Filtered distance data: %f, from sensor: %d", dis_data.distance, dis_data.sensor_id);
        }
    }
}


