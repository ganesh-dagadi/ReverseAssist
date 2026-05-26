#include "freertos/FreeRTOS.h"
#include "DistanceSensor.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"
#include "stdio.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

#define MOCK_SENSOR_TAG "Mock Sensor"
#define DISTANCE_SENSOR_TAG "Distance Sensor"
#define HC_SR04_ECHO_PIN 4
#define HC_SR04_TRIG_PIN 5
#define ISR_QUEUE_LENGTH 100
#define STATE_MACHINE_MEMORY 2048
#define DISTANCE_OUT_QUEUE_LENGTH 100

static QueueHandle_t echo_trig_queue;
static TaskHandle_t echo_event_reading_task;
QueueHandle_t distance_source_queue = NULL;  // actual definition

int mock_sensor_init(DistanceSensor*);
int mock_sensor_poll_distance(DistanceSensor*);
int hc_sr04_sensor_init(DistanceSensor* );
int hc_sr04_sensor_poll_distance(DistanceSensor*);
void sensor_distance_calculate_task(void* params);

struct DistanceSensor {
    int (*init)(DistanceSensor* self);
    int (*poll_distance)(DistanceSensor* self);
    void* sensor_data;
    short sensor_id;
};

typedef struct {
    short sensor_id;
    bool is_high;
    int64_t timestamp;
} IsrTrigData;


static void IRAM_ATTR echo_isr_handler(void *arg) {
    BaseType_t task_woken = pdFALSE;
    bool is_high = gpio_get_level(HC_SR04_ECHO_PIN);
    int64_t timestamp = esp_timer_get_time();
    IsrTrigData event;
    // default sensor id for now
    event.sensor_id = 0;
    event.is_high = is_high;
    event.timestamp = timestamp;

    xQueueSendFromISR(echo_trig_queue, &event, &task_woken);
    portYIELD_FROM_ISR(task_woken);
}

DistanceSensor* create_distance_sensor(int id) {
    DistanceSensor* sensor = malloc(sizeof(DistanceSensor));
    if (sensor == NULL) {
        ESP_LOGE(DISTANCE_SENSOR_TAG, "Failed to allocate memory for Distance sensor");
        return NULL;
    }
    switch (id) {
        case -1:
            sensor->init = mock_sensor_init;
            sensor->poll_distance = mock_sensor_poll_distance;
            sensor->sensor_data = NULL;
            break;
        case 0:
            sensor->init = hc_sr04_sensor_init;
            sensor->poll_distance = hc_sr04_sensor_poll_distance;
            sensor->sensor_data = NULL;
            break;
        default:
            ESP_LOGE(DISTANCE_SENSOR_TAG, "Unknown Distance sensor id");
            free(sensor);
            return NULL;
    }
    distance_source_queue = xQueueCreate(DISTANCE_OUT_QUEUE_LENGTH, sizeof(DistanceData));
    return sensor;
}

int init_distance_sensor(DistanceSensor* sensor) {
    return sensor->init(sensor);
}

int poll_distance(DistanceSensor* sensor) {
    return sensor->poll_distance(sensor);
}

void close_sensor(DistanceSensor* sensor) {
    ESP_LOGI(DISTANCE_SENSOR_TAG, "Trying to close sensor");
    if (sensor == NULL) {
        ESP_LOGE(DISTANCE_SENSOR_TAG, "Sensor is null. Cant close. Bye bye");
    }

    if (sensor->sensor_data != NULL) {
        ESP_LOGI(DISTANCE_SENSOR_TAG, "Freeing sensor Data");
        free(sensor->sensor_data);
    }
    ESP_LOGI(DISTANCE_SENSOR_TAG, "Freeing sensor");
    free(sensor);
    // stop the tasks and other resource
    if (echo_event_reading_task != NULL) {
        ESP_LOGI(DISTANCE_SENSOR_TAG, "Stopping echo reader task");
        vTaskDelete(echo_event_reading_task);
    }

    if (echo_trig_queue != NULL) {
        ESP_LOGI(DISTANCE_SENSOR_TAG, "Deleting echo reader queue");
        vQueueDelete(echo_trig_queue);
    }
    ESP_LOGI(DISTANCE_SENSOR_TAG, "Close sensor done - exiting");
}

void sleep_sensor(DistanceSensor* sensor) {
    ESP_LOGI(DISTANCE_SENSOR_TAG, "Trying to sleep the sensor");
    if (sensor == NULL) {
        ESP_LOGE(DISTANCE_SENSOR_TAG, "Sensor is NULL cant sleep");
        return;
    }
    // clear exisiting echo events queue
    ESP_LOGI(DISTANCE_SENSOR_TAG, "Clearing queue");
    xQueueReset(echo_trig_queue);
    ESP_LOGI(DISTANCE_SENSOR_TAG, "Pausing echo reading task");
    vTaskSuspend(echo_event_reading_task);
}

void resume_sensor(DistanceSensor* sensor) {
    ESP_LOGI(DISTANCE_SENSOR_TAG, "Resume echo reading task");
    vTaskResume(echo_event_reading_task);
}

int mock_sensor_init(DistanceSensor* self) {
    ESP_LOGI(MOCK_SENSOR_TAG, "Nothing to init");
    return 0;
}

int mock_sensor_poll_distance(DistanceSensor* self) {
    return 10;
}

int hc_sr04_sensor_init(DistanceSensor* sensor) {
    // Setup GPIO pins

    gpio_config_t gpio_config_trig = {0};

    gpio_config_trig.pin_bit_mask = (1ULL << HC_SR04_TRIG_PIN);
    gpio_config_trig.mode = GPIO_MODE_OUTPUT;
    gpio_config_trig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config_trig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config_trig.intr_type = GPIO_INTR_DISABLE;

    esp_err_t ret = gpio_config(&gpio_config_trig);

    if (ret != ESP_OK) {
        ESP_LOGE(DISTANCE_SENSOR_TAG,
                "Failed to configure TRIG pin for HC_SR04 sensor: %s",
                esp_err_to_name(ret));
        return -1;
    }

    //configure ECHO
    gpio_config_t gpio_config_echo = {0};

    gpio_config_echo.pin_bit_mask = (1ULL << HC_SR04_ECHO_PIN);
    gpio_config_echo.mode = GPIO_MODE_INPUT;
    gpio_config_echo.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config_echo.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config_echo.intr_type = GPIO_INTR_ANYEDGE;

    ret = gpio_config(&gpio_config_echo);

    if (ret != ESP_OK) {
        ESP_LOGE(DISTANCE_SENSOR_TAG,
                "Failed to configure ECHO pin for HC_SR04 sensor: %s",
                esp_err_to_name(ret));
        return -1;
    }

    // Configure interrupt service routine
    echo_trig_queue = xQueueCreate(ISR_QUEUE_LENGTH, sizeof(IsrTrigData));
    ret = gpio_install_isr_service(0);
    if (ret != ESP_OK) {
        ESP_LOGE(DISTANCE_SENSOR_TAG,
                "Failed to ISR for Echo pin: %s",
                esp_err_to_name(ret));
        return -1;
    }
    ret = gpio_isr_handler_add(HC_SR04_ECHO_PIN, echo_isr_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(DISTANCE_SENSOR_TAG,
                "Failed to add Handler to ISR for ECHO pin: %s",
                esp_err_to_name(ret));
        return -1;
    }

    // create a task to read the timestamp and calculate distance
    xTaskCreate(sensor_distance_calculate_task,
        "SENSOR_TRIG_DATA_READER_TASK",
         STATE_MACHINE_MEMORY,
         NULL,
         9, // high priority
         &echo_event_reading_task
    );

    return 0;
}
int hc_sr04_sensor_poll_distance(DistanceSensor* sensor) {
    // Ensure TRIG is LOW first
    gpio_set_level(HC_SR04_TRIG_PIN, 0);
    esp_rom_delay_us(2);

    // Send 10us pulse
    gpio_set_level(HC_SR04_TRIG_PIN, 1);
    esp_rom_delay_us(11);
    gpio_set_level(HC_SR04_TRIG_PIN, 0);
    return 0;
}


void sensor_distance_calculate_task(void* params) {
    uint8_t previous_edge = 0;
    uint64_t previous_time = 0L;
    int curr_distance = 0;
    while (1) {
        IsrTrigData event;
        if (xQueueReceive(echo_trig_queue, &event, 0) == pdPASS) {
            // WE GOT SOME DATA
            if (event.is_high) {
                if (previous_edge == 0) {
                    previous_edge = 1;
                    previous_time = event.timestamp;
                } else {
                    ESP_LOGW(DISTANCE_SENSOR_TAG, "Invalid High edge received. Updating new timestamp");
                    previous_time = event.timestamp;
                }
            } else {
                if (previous_edge == 1) {
                    uint64_t curr_time = event.timestamp;
                    // Long overflow handled by unsigned long
                    uint64_t time_delta = curr_time - previous_time;
                    curr_distance = (0.0343 * time_delta) / 2;
                    previous_edge = 0;
                } else {
                    ESP_LOGW(DISTANCE_SENSOR_TAG, "Invalid Low edge received. Resetting state");
                    previous_edge = 0;
                    previous_time = 0;
                }
            }
            // ESP_LOGI(DISTANCE_SENSOR_TAG, "Got Edge: %d, time: %lld, distance : %d\n",event.is_high, event.timestamp, curr_distance);
            DistanceData dis_data;
            dis_data.distance = curr_distance;
            dis_data.timestamp = previous_time; // we use the time at start of TRIG;
            xQueueSend(distance_source_queue, &dis_data, 20);
        }
        vTaskDelay(1);
    }
}






