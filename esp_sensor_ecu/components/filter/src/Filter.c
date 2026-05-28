#include "freertos/FreeRTOS.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "Filter.h"
#include "StateMachine.h"

#define MIN_DISTANCE 2
#define MAX_DISTANCE 400
#define MOVING_AVG_QUEUE_SIZE 5
#define SPIKE_THRESHOLD 100
#define SPIKE_COUNT_PASS 3
#define TAG "Filter"

typedef struct {
    int arr[MOVING_AVG_QUEUE_SIZE];
    int curr_sum;
    uint8_t left;
    uint8_t right;
} Queue;

static TaskHandle_t filter_task_handle = NULL;
FilterStrategy curr_strategy;
Queue moving_avg_queue;


int pop_queue() {
    if (moving_avg_queue.left == moving_avg_queue.right) {
        ESP_LOGW(TAG, "Moving average queue is empty");
        return -1;
    }
    moving_avg_queue.curr_sum -= moving_avg_queue.arr[moving_avg_queue.left];
    moving_avg_queue.left = (moving_avg_queue.left + 1) % MOVING_AVG_QUEUE_SIZE;
    return 0;
}

int push_queue(int distance) {
    if ((moving_avg_queue.right + 1)%MOVING_AVG_QUEUE_SIZE == moving_avg_queue.left) {
        pop_queue();
    }
    moving_avg_queue.arr[moving_avg_queue.right] = distance;
    moving_avg_queue.curr_sum += distance;
    moving_avg_queue.right = (moving_avg_queue.right + 1) % MOVING_AVG_QUEUE_SIZE;
    return 0;
}

int front_queue() {
    if (moving_avg_queue.left == moving_avg_queue.right) {
        ESP_LOGW(TAG, "Moving average queue is empty");
        return -1;
    }
    return moving_avg_queue.arr[moving_avg_queue.left];
}

int get_movin_average_queue() {
    if (moving_avg_queue.left == moving_avg_queue.right) return 0;
    uint8_t queue_size = 0;
    if (moving_avg_queue.left > moving_avg_queue.right) {
        queue_size = MOVING_AVG_QUEUE_SIZE - moving_avg_queue.left + moving_avg_queue.right;
    } else {
        queue_size = moving_avg_queue.right - moving_avg_queue.left;
    }
    return moving_avg_queue.curr_sum / queue_size; // left > right handled by unsiged int
}

void filter_distance_task(void*);

int set_filter_strategy(FilterStrategy strategy) {
    curr_strategy = strategy;
    return 0;
}

void start_distance_filter() {
    xTaskCreate(filter_distance_task,
        "DISTANCE_FILTER_TASK",
        2048,
        NULL,
        6,
        &filter_task_handle
    );
}

void stop_distance_filter() {
    if (filter_task_handle != NULL) {
        vTaskDelete(filter_task_handle);
    }
}

void filter_distance_task(void*) {
    int spike_count = 0;
    while (1) {
        DistanceData dis_data;
        if (xQueueReceive(distance_source_queue, &dis_data, 0) == pdPASS) {

            // filter out invalid readings
            if (dis_data.distance < MIN_DISTANCE || dis_data.distance > MAX_DISTANCE) {
                ESP_LOGW(TAG, "Invalid distance received : %d", dis_data.distance);
                continue;
            }

            // remove invalid spikes before they affect the moving average.
            // valid spikes when obstacle suddenly appears in front of the vehicle should be valid.
            int curr_moving_avg = get_movin_average_queue();
            if (abs(curr_moving_avg - dis_data.distance) > SPIKE_THRESHOLD) {
                ESP_LOGI(TAG, "Spike detected");
                spike_count++;
                if (spike_count > SPIKE_COUNT_PASS) {
                    spike_count = 0;
                    push_queue(dis_data.distance);
                }
            } else {
                spike_count = 0;
                push_queue(dis_data.distance);
            }

            // get filtered distance
            int filtered_distance = get_movin_average_queue();
            ESP_LOGI(TAG, "Filtered moving average: %d", filtered_distance);
        }
        vTaskDelay(2);
    }

}