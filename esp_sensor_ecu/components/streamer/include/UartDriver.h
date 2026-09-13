#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <stddef.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

int uart_driver_init(uart_port_t port,
                     int baud_rate,
                     gpio_num_t tx_pin,
                     gpio_num_t rx_pin,
                     int rx_buffer_size,
                     int tx_buffer_size);

void uart_driver_deinit(uart_port_t port);

int uart_driver_write(uart_port_t port, const uint8_t* data, size_t len);

int uart_driver_read(uart_port_t port, uint8_t* data, size_t len, TickType_t timeout_ticks);

int uart_driver_get_bytes_available(uart_port_t port, size_t* available_bytes);

#ifdef __cplusplus
}
#endif

#endif
