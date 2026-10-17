#pragma once
#include <cstddef>
#include <cstdint>

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

class DistancePayload {
    public:
    uint8_t sensor_id;
    float distance;
};

class StatusPayload {
    public:
    uint8_t sensor_id;
    int status;
};

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
    void (*payload_callback)(deserializer::MessageType, void*);
    uint8_t calculateCrc();
    void resetFrameState();

    size_t getRequiredFrameSize();
public:
    void registerPayloadCallback(void (*callback)(deserializer::MessageType type,
            void* payload));
    bool deserialize_data(uint8_t* buffer, size_t readSize);
};
