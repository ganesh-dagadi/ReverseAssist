#include "os.h"
#include "logger.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define TAG "OS"
int create_task(Os_TaskFunction task, const char* name, uint8_t priority, uint32_t size, Os_TaskHandle* handle) {
    log_info(TAG, "Starting FreeRTOS task %s \n", name);
    if (xTaskCreate(
        (TaskFunction_t)task,
        name,
        size,
        NULL,
        priority,
        (TaskHandle_t*)handle
    ) == pdPASS) return 0;

    log_error(TAG, "Error creating task \n");
    return -1;
}

int stop_task(Os_TaskHandle task) {
    log_info(TAG, "Stopping task \n");
    if (task != NULL) {
        vTaskDelete((TaskHandle_t)task);
    }
    return 0;
}

void sleep_task(uint32_t delay) {
    vTaskDelay(pdMS_TO_TICKS(delay));
}

int create_queue(uint8_t length, uint32_t size_of_ele, Os_QueueHandle* handle) {
    log_debug(TAG, "Creating queue \n");
    QueueHandle_t queue = xQueueCreate(
        length,
        size_of_ele
    );
    if (queue == NULL) {
        log_error(TAG, "Creating Queue failed from OS \n");
        return -1;
    }
    log_info(TAG, "Queue created successfully \n");
    if (handle != NULL) {
        *handle = (Os_QueueHandle) queue;
    }
    return 0;

}
int destory_queue(Os_QueueHandle handle) {
    log_info(TAG, "Deleting Queue \n");
    vQueueDelete((QueueHandle_t) handle);
    return 0;
}

int push_queue(Os_QueueHandle handle, void* data) {
    if (handle == NULL) {
        log_error(TAG, "Queue handle is null. Unable to push");
        return -1;
    }
    if (xQueueSend(handle, data, 0) == pdPASS) {
        return 0;
    } else return -1;
}

int suspend_task(Os_TaskHandle task) {
    if (task == NULL) {
        log_error(TAG, "Task handle is null. Unable to suspend");
        return -1;
    }
    vTaskSuspend((TaskHandle_t)task);
    return 0;
}

int resume_task(Os_TaskHandle task) {
    if (task == NULL) {
        log_error(TAG, "Task handle is null. Unable to resume");
        return -1;
    }
    vTaskResume((TaskHandle_t)task);
    return 0;
}

int poll_queue_blocking(Os_QueueHandle handle, void* data) {
    if (handle == NULL) {
        log_error(TAG, "Queue handle is null. Unable to poll");
        return -1;
    }

    if (xQueueReceive(handle, data, portMAX_DELAY) == pdPASS) {
        return 0;
    } else return -1;
}