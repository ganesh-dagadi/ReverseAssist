#include "serializer.h"
#include "string.h"

#define SENSOR_ID_KEY "sensor_id"
#define DISTANCE_KEY "distance"
#define STATUS_KEY "status"

static const uint8_t VERSION = 1;

size_t calculate_header_size() {
    return 1 + // version
    1 + // length of payload
    1; // options
}

size_t calculate_payload_size(const DistanceData* data) {
    size_t sensor_id_len = snprintf(NULL, 0, "%u", data->sensor_id);
    size_t distance_len = snprintf(NULL, 0, "%.2f", data->distance);

    return
    strlen(SENSOR_ID_KEY) +
    1 + // =
    sensor_id_len + 
    1 + // ;
    strlen(DISTANCE_KEY)+
    1 + // =
    distance_len;
}

size_t calculate_status_payload_size(const SensorStatusData* data) {
    size_t sensor_id_len = snprintf(NULL, 0, "%u", data->sensor_id);
    size_t status_len = snprintf(NULL, 0, "%d", data->status);

    return
    strlen(SENSOR_ID_KEY) +
    1 + // =
    sensor_id_len +
    1 + // ;
    strlen(STATUS_KEY) +
    1 + // =
    status_len;
}

void append_crc(uint8_t* buffer, size_t buf_len)
{
    uint8_t crc = 0xFF;

    for (size_t i = 0; i < buf_len; ++i)
    {
        crc ^= buffer[i];

        for (int bit = 0; bit < 8; ++bit)
        {
            if (crc & 0x80)
                crc = (uint8_t)((crc << 1) ^ 0x1D);
            else
                crc = (uint8_t)(crc << 1);
        }
    }

    buffer[buf_len] = crc ^ 0xFF;
}

void serialize_data(const DistanceData* data, uint8_t* buffer, size_t payload_len) {
    uint8_t* cursor = buffer;
    // write the header
    *cursor++ = VERSION;
    *cursor++ = payload_len;
    *cursor++ = 0; //options

    // write the payload
    char payload[payload_len + 1]; //+1 for \0
    snprintf(payload, payload_len + 1, SENSOR_ID_KEY"=%u;"DISTANCE_KEY"=%.2f", data->sensor_id, data->distance);
    memcpy(cursor, payload, payload_len);
    cursor += payload_len;
    append_crc(buffer, calculate_header_size() + payload_len);
}

void serialize_status_data(const SensorStatusData* data, uint8_t* buffer, size_t payload_len) {
    uint8_t* cursor = buffer;
    *cursor++ = VERSION;
    *cursor++ = payload_len;
    *cursor++ = 0; // options

    char payload[payload_len + 1];
    snprintf(payload, payload_len + 1, SENSOR_ID_KEY"=%u;"STATUS_KEY"=%d", data->sensor_id, data->status);
    memcpy(cursor, payload, payload_len);
    cursor += payload_len;
    append_crc(buffer, calculate_header_size() + payload_len);
}