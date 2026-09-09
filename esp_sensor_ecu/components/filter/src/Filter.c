#include "esp_log.h"
#include "Filter.h"
#include "os.h"
#include <math.h>
#include <stdlib.h>

#define MIN_DISTANCE 2
#define MAX_DISTANCE 400
#define MOVING_AVG_QUEUE_SIZE 5
#define SPIKE_THRESHOLD 100
#define SPIKE_COUNT_PASS 3
#define TAG "Filter"

typedef struct {
    float arr[MOVING_AVG_QUEUE_SIZE];
    float curr_sum;
    uint8_t left;
    uint8_t right;
    uint8_t count;
} Queue;

static Os_TaskHandle filter_task_handle = NULL;
static FilterStrategy curr_strategy;
static Queue *moving_avg_queues = NULL;
static uint8_t *spike_counts = NULL;
static Os_QueueHandle distance_source_queue = NULL;
static Os_QueueHandle filtered_distance_queue = NULL;
static int sensor_count = 0;


static int moving_avg_pop(Queue *queue) {
    if (queue->count == 0) {
        ESP_LOGW(TAG, "Moving average queue is empty");
        return -1;
    }
    queue->curr_sum -= queue->arr[queue->left];
    queue->left = (queue->left + 1) % MOVING_AVG_QUEUE_SIZE;
    queue->count--;
    return 0;
}

static int moving_avg_push(Queue *queue, float distance) {
    if (queue->count == MOVING_AVG_QUEUE_SIZE) {
        moving_avg_pop(queue);
    }
    queue->arr[queue->right] = distance;
    queue->curr_sum += distance;
    queue->right = (queue->right + 1) % MOVING_AVG_QUEUE_SIZE;
    queue->count++;
    return 0;
}

static float get_moving_average(const Queue *queue) {
    if (queue->count == 0) return 0;
    return queue->curr_sum / queue->count;
}

void filter_distance_task(void*);

int set_filter_strategy(FilterStrategy strategy) {
    curr_strategy = strategy;
    return 0;
}

int set_sensor_count(int count) {
    if (count <= 0 || filter_task_handle != NULL) {
        return -1;
    }

    Queue *new_queues = calloc((size_t)count, sizeof(*new_queues));
    uint8_t *new_spike_counts = calloc((size_t)count, sizeof(*new_spike_counts));
    if (new_queues == NULL || new_spike_counts == NULL) {
        free(new_queues);
        free(new_spike_counts);
        return -1;
    }

    free(moving_avg_queues);
    free(spike_counts);
    moving_avg_queues = new_queues;
    spike_counts = new_spike_counts;
    sensor_count = count;
    return 0;
}

int start_distance_filter(Os_QueueHandle source_queue, Os_QueueHandle output_queue) {
    if (source_queue == NULL || output_queue == NULL || filter_task_handle != NULL ||
        moving_avg_queues == NULL || spike_counts == NULL || sensor_count <= 0) {
        return -1;
    }

    distance_source_queue = source_queue;
    filtered_distance_queue = output_queue;

    for (int sensor_id = 0; sensor_id < sensor_count; sensor_id++) {
        moving_avg_queues[sensor_id] = (Queue){0};
        spike_counts[sensor_id] = 0;
    }
    return create_task(filter_distance_task, "DISTANCE_FILTER_TASK", 6, 2048,
                       &filter_task_handle);
}

int stop_distance_filter() {
    if (filter_task_handle != NULL) {
        stop_task(filter_task_handle);
        filter_task_handle = NULL;
    }
    distance_source_queue = NULL;
    filtered_distance_queue = NULL;
    free(moving_avg_queues);
    free(spike_counts);
    moving_avg_queues = NULL;
    spike_counts = NULL;
    sensor_count = 0;
    return 0;
}

void filter_distance_task(void*) {
    while (1) {
        DistanceData dis_data;
        if (poll_queue_blocking(distance_source_queue, &dis_data) == 0) {
            if (dis_data.sensor_id >= sensor_count) {
                ESP_LOGW(TAG, "Invalid sensor id: %d", dis_data.sensor_id);
                continue;
            }

            Queue *moving_avg_queue = &moving_avg_queues[dis_data.sensor_id];

            // filter out invalid readings
            if (dis_data.distance < MIN_DISTANCE || dis_data.distance > MAX_DISTANCE) {
                ESP_LOGW(TAG, "Invalid distance received : %f", dis_data.distance);
                continue;
            }

            // remove invalid spikes before they affect the moving average.
            // valid spikes when obstacle suddenly appears in front of the vehicle should be valid.
            float curr_moving_avg = get_moving_average(moving_avg_queue);
            if (fabsf(curr_moving_avg - dis_data.distance) > SPIKE_THRESHOLD) {
                ESP_LOGI(TAG, "Spike detected");
                spike_counts[dis_data.sensor_id]++;
                if (spike_counts[dis_data.sensor_id] > SPIKE_COUNT_PASS) {
                    spike_counts[dis_data.sensor_id] = 0;
                    moving_avg_push(moving_avg_queue, dis_data.distance);
                }
                continue;
            } else {
                spike_counts[dis_data.sensor_id] = 0;
                moving_avg_push(moving_avg_queue, dis_data.distance);
            }

            // get filtered distance
            float filtered_distance = get_moving_average(moving_avg_queue);
            DistanceData filtered_data;
            filtered_data.sensor_id = dis_data.sensor_id;
            filtered_data.distance = filtered_distance;
            push_queue(filtered_distance_queue, &filtered_data);
        }
    }

}