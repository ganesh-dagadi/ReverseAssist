#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>

namespace deserializer {
    enum MessageType {
        DISTANCE_DATA,
        STATUS_DATA
    };

    enum State {
        NO_SOF,
        READ_HEADER,
        READ_PAYLOAD,
        CALCULATE_CRC
    };
};


typedef struct {
	uint8_t sof;
	uint8_t version;
	uint8_t payload_length;
	uint8_t options;
	deserializer::MessageType type;
	union {
		struct {
			unsigned int sensor_id;
			float distance;
		} distance;
		struct {
			unsigned int sensor_id;
			int status;
		} status;
	} data;
	uint8_t crc;
} MessageFrame;

class Deserializer {
private:
    deserializer::State mState = deserializer::NO_SOF;
    uint8_t mBufIdx = 0; // track till where the buffer was read
    uint8_t* mFrameData;
    uint8_t mFrameIdx = 0;
    uint8_t mPayloadRead = 0;
    int8_t mHeaderVersion = -1;
    int8_t mPayloadLength = 0;
    int8_t mOptions = -1;
    std::function<void(deserializer::MessageType, MessageFrame)> payload_callback = NULL;
    uint8_t calculateCrc();
    void resetFrameState();

    size_t getRequiredFrameSize();
public:
    void registerPayloadCallback(std::function<void(deserializer::MessageType, MessageFrame)>);
    bool deserialize_data(uint8_t* buffer, size_t readSize);
};
