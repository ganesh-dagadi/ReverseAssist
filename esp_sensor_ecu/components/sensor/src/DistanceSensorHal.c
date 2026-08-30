#include "IDistanceSensorHal.h"
#include "freertos/FreeRTOS.h"
#include "logger.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "os.h"

#define TAG "DistanceSensorHAL"

typedef struct {
    uint8_t trig_pin;
    uint8_t echo_pin;
}SensorConfig;

typedef struct {
    uint64_t timestamp;
    bool is_high;
} IsrData;

typedef struct {
    SensorConfig sensors[1];
} UltrasonicConfig;

UltrasonicConfig mConfig;
QueueHandle_t isr_queue;
TaskHandle_t driver_task_handle;
void (*distance_callback_to_client)(int, float) = NULL;
void (*status_handler)(int,int) = NULL;

int getSensorListLength() {
    return sizeof(mConfig.sensors) / sizeof (mConfig.sensors[0]);
}
void distance_driver_task(void *args) {
    while (1) {
        for (int currSensor = 0; currSensor < getSensorListLength(); currSensor++) {
            SensorConfig sensor = mConfig.sensors[currSensor];

            gpio_set_level(sensor.trig_pin, 0);
            vTaskDelay(pdMS_TO_TICKS(2));

            gpio_set_level(sensor.trig_pin, 1);
            esp_rom_delay_us(11);
            gpio_set_level(sensor.trig_pin, 0);

            IsrData data;
            bool is_prev_high = 0;
            uint64_t prev_high = 0;
            while (true) {
                if (xQueueReceive(isr_queue, &data, pdMS_TO_TICKS(50)) == pdTRUE) {
                    if (data.is_high){
                        if (is_prev_high) {
                            log_error(TAG, "Two consecutive high for sensor: %d. skipping measurement", currSensor);
                            break;
                        } else {
                            is_prev_high = true;
                            prev_high = data.timestamp;
                        }
                    } else {
                        if (is_prev_high != 1) {
                            log_error(TAG, "Two consecutive low for sensor: %d. skipping measurement", currSensor);
                            break;
                        } else {
                            uint64_t timediff = data.timestamp - prev_high;
                            float distance = (0.0343 * timediff) / 2;
                            log_info(TAG, "Distance: %f", distance);
                            break;
                        }
                    }
                    
                } else {
                    log_error(TAG, "Sensor %d did not receive an echo pulse", currSensor);
                }
            }
            
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

int setup_driver() {
    log_info(TAG, "Setting up Distance sensor driver");
    SensorConfig sensor_one;
    sensor_one.trig_pin = 5;
    sensor_one.echo_pin = 4;
    mConfig.sensors[0] = sensor_one;
    gpio_install_isr_service(0);
    isr_queue = xQueueCreate(10, sizeof(IsrData)); // hold a single timestamp
    
    if (xTaskCreate(
        distance_driver_task,
        "DISTANCE_DRIVER_TASK",
        2048,
        NULL,
        8,
        &driver_task_handle
    ) != pdPASS) {
        log_error(TAG, "Error creating task \n");
        return -1;
    }
    // Start suspended — streaming will resume the task when requested
    suspend_task((Os_TaskHandle)driver_task_handle);
    log_info(TAG, "Driver setup done");
    return 0;
}

static void IRAM_ATTR echo_isr_handler(void *arg) {
    int64_t now = esp_timer_get_time();
    gpio_num_t pin = (gpio_num_t)(uintptr_t)arg;
    bool is_high = gpio_get_level(pin);
    BaseType_t higher_priority_task_woken = pdFALSE;
    IsrData data;
    data.timestamp = now;
    data.is_high = is_high;
    xQueueSendFromISR(
        isr_queue,
        &data,
        &higher_priority_task_woken
    );

    if (higher_priority_task_woken) {
        portYIELD_FROM_ISR();
    }
}

int open_sensor(int sensor_id) {
    log_info(TAG, "Opening sensor with id %d", sensor_id);
    SensorConfig sensor = mConfig.sensors[sensor_id];

    // setup gpio input and output
    gpio_config_t gpio_config_trig = {0};

    gpio_config_trig.pin_bit_mask = (1ULL << sensor.trig_pin);
    gpio_config_trig.mode = GPIO_MODE_OUTPUT;
    gpio_config_trig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config_trig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config_trig.intr_type = GPIO_INTR_DISABLE;

    esp_err_t ret = gpio_config(&gpio_config_trig);

    if (ret != ESP_OK) {
        log_error(TAG,
                "Failed to configure TRIG pin for HC_SR04 sensor with id %d: %s",sensor_id,
                esp_err_to_name(ret));
        return -1;
    }

    //configure ECHO
    gpio_config_t gpio_config_echo = {0};

    gpio_config_echo.pin_bit_mask = (1ULL << sensor.echo_pin);
    gpio_config_echo.mode = GPIO_MODE_INPUT;
    gpio_config_echo.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config_echo.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config_echo.intr_type = GPIO_INTR_ANYEDGE;

    ret = gpio_config(&gpio_config_echo);

    if (ret != ESP_OK) {
        log_error(TAG,
                "Failed to configure ECHO pin for HC_SR04 sensor with id %d: %s",sensor_id,
                esp_err_to_name(ret));
        return -1;
    }

    // setup interrupt

    ret = gpio_isr_handler_add(sensor.echo_pin, echo_isr_handler, (void *)(uintptr_t)sensor.echo_pin);
    if (ret != ESP_OK) {
        log_error(TAG,
                "Failed to add Handler to ISR for ECHO pin: %s",
                esp_err_to_name(ret));
        return -1;
    }

    log_info(TAG, "openSensor for id %d done", sensor_id);
    return 0;
}

int close_sensor(int sensor_id) {
    log_info(TAG, "Closing sensor with id %d", sensor_id);
    return 0;
}

int register_distance_callback(void (*distance_callback_from_client)(int,float)) {
    log_info(TAG, "Registering Distance callback");
    distance_callback_to_client = distance_callback_from_client;
    return 0;
}

int unregister_distance_callback() {
    log_info(TAG, "Unregistering distance callback");
    return 0;
}

int register_status_callback(void (*status_callback)(int, int)) {
    log_info(TAG, "Registering status callback");
    return 0;
}

int unregister_status_callback() {
    log_info(TAG, "Unregistering status callback");
    return 0;
}

int stream_distance() {
    if (driver_task_handle == NULL) {
        log_error(TAG, "Driver task handle NULL. Cannot start streaming");
        return -1;
    }
    resume_task((Os_TaskHandle)driver_task_handle);
    log_info(TAG, "Started distance streaming");
    return 0;
}

int stop_stream_distance() {
    if (driver_task_handle == NULL) {
        log_error(TAG, "Driver task handle NULL. Cannot stop streaming");
        return -1;
    }
    suspend_task((Os_TaskHandle)driver_task_handle);
    log_info(TAG, "Stopped distance streaming");
    return 0;
}

