#include "freertos/FreeRTOS.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "Filter.h"

static TaskHandle_t filter_task_handle = NULL;
FilterStrategy curr_strategy;

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

    while (1) {
        DistanceData dis_data;
        if (xQueueReceive(distance_source_queue, &dis_data, 0) == pdPASS) {
            ESP_LOGI("Filter", "Distance: %d, Time: %lld \n", dis_data.distance, dis_data.timestamp);
        }
        vTaskDelay(2);
    }

}