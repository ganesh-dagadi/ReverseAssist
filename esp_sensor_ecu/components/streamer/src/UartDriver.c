#include "UartDriver.h"

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/queue.h"

#define TAG "UartDriver"

static const uart_config_t default_uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
};

int uart_driver_init(uart_port_t port,
                     int baud_rate,
                     gpio_num_t tx_pin,
                     gpio_num_t rx_pin,
                     int rx_buffer_size,
                     int tx_buffer_size) {
    uart_config_t cfg = default_uart_config;
    cfg.baud_rate = baud_rate;

    esp_err_t err = uart_driver_install(port,
                                       rx_buffer_size,
                                       tx_buffer_size,
                                       0,
                                       NULL,
                                       0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "uart_driver_install failed: %s", esp_err_to_name(err));
        return -1;
    }

    err = uart_param_config(port, &cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "uart_param_config failed: %s", esp_err_to_name(err));
        uart_driver_delete(port);
        return -1;
    }

    err = uart_set_pin(port, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "uart_set_pin failed: %s", esp_err_to_name(err));
        uart_driver_delete(port);
        return -1;
    }

    return 0;
}

void uart_driver_deinit(uart_port_t port) {
    uart_driver_delete(port);
}

int uart_driver_write(uart_port_t port, const uint8_t* data, size_t len) {
    if (data == NULL || len == 0) {
        return 0;
    }

    int written = uart_write_bytes(port, (const char*)data, len);
    if (written < 0) {
        ESP_LOGE(TAG, "uart_write_bytes failed on port %d", port);
        return -1;
    }

    return written;
}

int uart_driver_read(uart_port_t port, uint8_t* data, size_t len, TickType_t timeout_ticks) {
    if (data == NULL || len == 0) {
        return 0;
    }

    int read_len = uart_read_bytes(port, data, len, timeout_ticks);
    if (read_len < 0) {
        ESP_LOGE(TAG, "uart_read_bytes failed on port %d", port);
        return -1;
    }

    return read_len;
}

int uart_driver_get_bytes_available(uart_port_t port, size_t* available_bytes) {
    if (available_bytes == NULL) {
        return -1;
    }

    size_t bytes = 0;
    esp_err_t err = uart_get_buffered_data_len(port, &bytes);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "uart_get_buffered_data_len failed: %s", esp_err_to_name(err));
        return -1;
    }

    *available_bytes = bytes;
    return 0;
}
