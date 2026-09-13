#include "Streamer.h"
#include "logger.h"
#include "serializer.h"
#include "UartDriver.h"

#define TAG "Uart Streamer"
#define UART_TX_QUEUE_LENGTH 20
#define UART_TX_PACKET_SIZE 128
#define UART_PORT_NUM UART_NUM_2
#define UART_BAUD_RATE 115200
#define UART_TX_PIN GPIO_NUM_17
#define UART_RX_PIN GPIO_NUM_16
#define UART_RX_BUFFER_SIZE 1024
#define UART_TX_BUFFER_SIZE 1024

static Os_TaskHandle status_streamer_task_handle = NULL;
static Os_TaskHandle distance_streamer_task_handle = NULL;
static Os_TaskHandle uart_tx_task_handle = NULL;
static Os_TaskHandle uart_rx_task_handle = NULL;
static Os_QueueHandle filtered_distance_queue = NULL;
static Os_QueueHandle status_queue = NULL;
static Os_QueueHandle uart_tx_queue = NULL;
static uint8_t PROTOCOL_VERSION_ONE = 1;

/* 
Stream packet protocol

Header          |  Payload                          | CRC
=== === =======  === === === === === === === ===    ====
ver p.len option|  size in bytes mentioned in header|
1   1     1      |  key=value;key=value              |   
=== === =======  === === === === === === ===  ===    ====
*/

void status_streamer_task_runnable(void*);
void distance_streamer_task_runnable(void*);
void uart_tx_task_runnable(void*);
void uart_rx_task_runnable(void*);
void dump_buffer(uint8_t* buf, size_t len);

typedef struct {
    uint8_t* buf;
    size_t len;
} StreamData;

int start_streamer(Os_QueueHandle input_queue, Os_QueueHandle status_input_queue) {
    if (input_queue == NULL || status_input_queue == NULL || status_streamer_task_handle != NULL || distance_streamer_task_handle != NULL) {
        return -1;
    }

    filtered_distance_queue = input_queue;
    status_queue = status_input_queue;

    if (create_queue(UART_TX_QUEUE_LENGTH, sizeof(StreamData), &uart_tx_queue) != 0) {
        log_error(TAG, "Failed to create UART TX queue");
        return -1;
    }

    if (create_task(status_streamer_task_runnable, "UART_STATUS_STREAMER_TASK", 6, 2048,
                   &status_streamer_task_handle) != 0) {
        log_error(TAG, "Failed to start status streamer task");
        return -1;
    }

    if (create_task(distance_streamer_task_runnable, "UART_DISTANCE_STREAMER_TASK", 5, 2048,
                   &distance_streamer_task_handle) != 0) {
        log_error(TAG, "Failed to start distance streamer task");
        return -1;
    }

    if (create_task(uart_tx_task_runnable, "UART_TX_TASK", 7, 2048,
                   &uart_tx_task_handle) != 0) {
        log_error(TAG, "Failed to start UART TX task");
        return -1;
    }

    if (uart_driver_init(UART_PORT_NUM,
                        UART_BAUD_RATE,
                        UART_TX_PIN,
                        UART_RX_PIN,
                        UART_RX_BUFFER_SIZE,
                        UART_TX_BUFFER_SIZE) != 0) {
        log_error(TAG, "Failed to initialize UART driver");
        return -1;
    }

    if (create_task(uart_rx_task_runnable, "UART_RX_TASK", 6, 2048,
                   &uart_rx_task_handle) != 0) {
        log_error(TAG, "Failed to start UART RX task");
        uart_driver_deinit(UART_PORT_NUM);
        return -1;
    }

    return 0;
}

static void enqueue_serialized_packet(uint8_t* packet, size_t data_len) {
    if (packet == NULL || uart_tx_queue == NULL) {
        free(packet);
        return;
    }
    StreamData data;
    data.buf = packet;
    data.len = data_len;
    if (push_queue(uart_tx_queue, &data) != 0) {
        log_error(TAG, "Unable to enqueue serialized packet to UART TX queue");
        free(packet);
    }
}

void status_streamer_task_runnable(void*) {
    while (1) {
        SensorStatusData status_data;
        if (poll_queue_blocking(status_queue, &status_data) == 0) {
            log_info(TAG, "Status update: status=%d from sensor=%d", status_data.status, status_data.sensor_id);

            size_t payload_size = calculate_status_payload_size(&status_data);
            size_t header_size = calculate_header_size();
            size_t total_packet_size = header_size + payload_size + 1;
            uint8_t* buf = (uint8_t*) malloc(sizeof(uint8_t) * total_packet_size);
            if (!buf) {
                log_error(TAG, "Unable to allocate buffer for status packet");
                continue;
            }
            serialize_status_data(&status_data, buf, payload_size);
            enqueue_serialized_packet(buf, total_packet_size);
        }
    }
}

void distance_streamer_task_runnable(void*) {
    while (1) {
        DistanceData dis_data;
        if (poll_queue_blocking(filtered_distance_queue, &dis_data) == 0) {
            log_info(TAG, "Filtered distance data: %f, from sensor: %d", dis_data.distance, dis_data.sensor_id);
            size_t payload_size = calculate_payload_size(&dis_data);
            size_t header_size = calculate_header_size();
            size_t total_packet_size = header_size + payload_size + 1; //1 byte CRC
            uint8_t* buf = (uint8_t*) malloc(sizeof(uint8_t) * total_packet_size);
            if (!buf) {
                log_error(TAG, "Unable to allocate buffer for Data packet");
                continue;
            }
            serialize_data(&dis_data, buf, payload_size);
            enqueue_serialized_packet(buf, total_packet_size);
        }
    }
}

void uart_tx_task_runnable(void*) {
    while (1) {
        StreamData data;
        if (poll_queue_blocking(uart_tx_queue, &data) == 0) {
            dump_buffer(data.buf, data.len);
            if (uart_driver_write(UART_PORT_NUM, data.buf, data.len) < 0) {
                log_error(TAG, "UART write failed");
            }
            free(data.buf);
        }
    }
}

void uart_rx_task_runnable(void*) {
    uint8_t rx_buf[64];
    while (1) {
        size_t available = 0;
        if (uart_driver_get_bytes_available(UART_PORT_NUM, &available) == 0 && available > 0) {
            size_t to_read = (available > sizeof(rx_buf)) ? sizeof(rx_buf) : available;
            int received = uart_driver_read(UART_PORT_NUM, rx_buf, to_read, 0);
            if (received > 0) {
                log_info(TAG, "UART RX: received %d bytes", received);
                // TODO: parse protocol frames here when command protocol is implemented
            }
        }
    }
}

void dump_buffer(uint8_t* buf, size_t len) {
        log_dump(TAG, buf, len);
}

