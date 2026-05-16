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

static QueueHandle_t echo_trig_queue;
static TaskHandle_t echo_event_reading_task;

int mock_sensor_init(DistanceSensor*);
int mock_sensor_poll_distance(DistanceSensor*);
int hc_sr04_sensor_init(DistanceSensor* );
int hc_sr04_sensor_poll_distance(DistanceSensor*);
void sensor_distance_calcule_task(void* params);

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
} Isr_Trig_Data;


static void IRAM_ATTR echo_isr_handler(void *arg) {
    BaseType_t task_woken = pdFALSE;
    bool is_high = gpio_get_level(HC_SR04_ECHO_PIN);
    int64_t timestamp = esp_timer_get_time();
    Isr_Trig_Data event;
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
            break;
        default:
            ESP_LOGE(DISTANCE_SENSOR_TAG, "Unknown Distance sensor id");
            free(sensor);
            return NULL;
    }
    return sensor;
}

int init_distance_sensor(DistanceSensor* sensor) {
    return sensor->init(sensor);
}

int poll_distance(DistanceSensor* sensor) {
    return sensor->poll_distance(sensor);
}

void close_sensor(DistanceSensor* sensor) {
    //TODO:free pointers held by sensor too
    free(sensor);
    // stop the tasks
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
    echo_trig_queue = xQueueCreate(ISR_QUEUE_LENGTH, sizeof(Isr_Trig_Data));
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
    xTaskCreate(sensor_distance_calcule_task,
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

void sensor_distance_calcule_task(void* params) {
    while (1) {
        Isr_Trig_Data event;
        if (xQueueReceive(echo_trig_queue, &event, 0) == pdPASS) {
            // WE GOT SOME DATA
            ESP_LOGI(DISTANCE_SENSOR_TAG, "Got Edge: %d, time: %lld \n",event.is_high, event.timestamp);
        }
        vTaskDelay(pdMS_TO_TICKS(75));
    }
}






