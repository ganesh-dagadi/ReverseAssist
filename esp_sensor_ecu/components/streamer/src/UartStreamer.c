#include "freertos/FreeRTOS.h"
#include "Streamer.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define TAG "Uart Streamer"


static TaskHandle_t streamer_task_handle = NULL;

void streamer_task(void*);

int start_streamer() {
    xTaskCreate(streamer_task,
        "UART_STREAMER_TASK",
        2048,
        NULL,
        5,
        &streamer_task_handle
    );
    return 0;
}

void streamer_task(void*) {
    while (1) {
        DistanceData dis_data;
        if (xQueueReceive(filtered_distance_queue, &dis_data, 0) == pdPASS) {
            ESP_LOGI(TAG, "Filtered distance data: %d", dis_data.distance);
        }
        vTaskDelay(2);
    }
}


