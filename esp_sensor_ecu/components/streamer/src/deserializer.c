#include "deserializer.h"

#include <string.h>

#define SOF 0x40
#define HEADER_SIZE 3
#define STATE_WAIT_FOR_SOF 0
#define STATE_READ_HEADER 1
#define STATE_READ_PAYLOAD 2
#define STATE_READ_CRC 3

static uint8_t calculate_crc(const uint8_t *buffer, size_t length)
{
    uint8_t crc = 0xFF;

    for (size_t index = 0; index < length; ++index) {
        crc ^= buffer[index];
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x1D)
                                : (uint8_t)(crc << 1);
        }
    }

    return crc ^ 0xFF;
}

static void reset_parser(Deserializer *deserializer)
{
    deserializer->state = STATE_WAIT_FOR_SOF;
    deserializer->header_index = 0;
    deserializer->payload_length = 0;
    deserializer->payload_index = 0;
}

static void parse_command(Deserializer *deserializer)
{
    static const char start_command[] = "command=start";
    static const char stop_command[] = "command=stop";
    const uint8_t *payload = deserializer->frame + 1 + HEADER_SIZE;

    if (deserializer->command_callback == NULL) {
        return;
    }

    if (deserializer->payload_length == sizeof(start_command) - 1 &&
        memcmp(payload, start_command, sizeof(start_command) - 1) == 0) {
        deserializer->command_callback(DISTANCE_SENSOR_CMD_RESUME);
    } else if (deserializer->payload_length == sizeof(stop_command) - 1 &&
               memcmp(payload, stop_command, sizeof(stop_command) - 1) == 0) {
        deserializer->command_callback(DISTANCE_SENSOR_CMD_PAUSE);
    }
}

void deserializer_init(Deserializer *deserializer,
                       DeserializerCommandCallback command_callback)
{
    memset(deserializer, 0, sizeof(*deserializer));
    deserializer->command_callback = command_callback;
}

void deserializer_process(Deserializer *deserializer,
                          const uint8_t *data, size_t length)
{
    for (size_t index = 0; index < length; ++index) {
        uint8_t byte = data[index];

        if (deserializer->state == STATE_WAIT_FOR_SOF) {
            if (byte == SOF) {
                deserializer->frame[0] = byte;
                deserializer->header_index = 0;
                deserializer->state = STATE_READ_HEADER;
            }
            continue;
        }

        if (deserializer->state == STATE_READ_HEADER) {
            deserializer->frame[1 + deserializer->header_index] = byte;
            if (deserializer->header_index == 1) {
                deserializer->payload_length = byte;
                // if (byte > DESERIALIZER_MAX_PAYLOAD) {
                //     reset_parser(deserializer);
                //     continue;
                // }
            }
            ++deserializer->header_index;
            if (deserializer->header_index == HEADER_SIZE) {
                deserializer->payload_index = 0;
                deserializer->state = deserializer->payload_length == 0
                                          ? STATE_READ_CRC
                                          : STATE_READ_PAYLOAD;
            }
            continue;
        }

        if (deserializer->state == STATE_READ_PAYLOAD) {
            deserializer->frame[1 + HEADER_SIZE + deserializer->payload_index] = byte;
            ++deserializer->payload_index;
            if (deserializer->payload_index == deserializer->payload_length) {
                deserializer->state = STATE_READ_CRC;
            }
            continue;
        }

        deserializer->frame[1 + HEADER_SIZE + deserializer->payload_length] = byte;
        if (calculate_crc(deserializer->frame,
                          1 + HEADER_SIZE + deserializer->payload_length) == byte) {
            parse_command(deserializer);
        }
        reset_parser(deserializer);
    }
}