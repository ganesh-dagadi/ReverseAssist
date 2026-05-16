#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "StateMachine.h"

#define STATE_MACHINE_MEMORY 2048

const char* TAG = "Main";
void app_main(void) {
    ESP_LOGI(TAG , "Started Main \n");
    if (init_state_machine() == -1) {
        ESP_LOGE(TAG , "Initializing State machine failed \n");
        return;
    }
    ESP_LOGI(TAG, "Starting Main State Machine");
    xTaskCreate(start_state_machine,
        "STATE_MACHINE_TASK",
         STATE_MACHINE_MEMORY,
         NULL,
         5, // medium priority
         NULL
    );

    ESP_LOGI(TAG , "Done. Exiting Main \n");
}