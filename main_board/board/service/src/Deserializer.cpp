#include "Deserializer.hpp"
#include "Logging.hpp"
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SOF 0x40
#define MAX_PAYLOAD 255
#define SENSOR_ID_KEY "sensor_id"
#define DISTANCE_KEY "distance"
#define STATUS_KEY "status"


static void print_frame(const MessageFrame *frame)
{
    std::string message =
        "version=" + std::to_string(frame->version) +
        " options=" + std::to_string(frame->options) + " ";

    if (frame->type == deserializer::DISTANCE_DATA) {
        message +=
            "sensor_id=" + std::to_string(frame->data.distance.sensor_id) +
            " distance=" + std::to_string(frame->data.distance.distance);
    } else {
        message +=
            "sensor_id=" + std::to_string(frame->data.status.sensor_id) +
            " status=" + std::to_string(frame->data.status.status);
    }

    Logger::getInstance().debug(message);
}

static bool parse_payload(const uint8_t *payload, size_t payload_length,
				 MessageFrame *frame)
{
	char text[MAX_PAYLOAD + 1];
	int consumed;

	if (payload_length > MAX_PAYLOAD) {
		return false;
	}

	memcpy(text, payload, payload_length);
	text[payload_length] = '\0';

	if (sscanf(text, SENSOR_ID_KEY"=%u;"DISTANCE_KEY"=%f%n",
		   &frame->data.distance.sensor_id,
		   &frame->data.distance.distance, &consumed) == 2 &&
	    (size_t)consumed == payload_length) {
		frame->type = deserializer::DISTANCE_DATA;
		return true;
	}

	if (sscanf(text, SENSOR_ID_KEY"=%u;"STATUS_KEY"=%d%n",
		   &frame->data.status.sensor_id,
		   &frame->data.status.status, &consumed) == 2 &&
	    (size_t)consumed == payload_length) {
		frame->type = deserializer::STATUS_DATA;
		return true;
	}

	return false;
}


size_t Deserializer::getRequiredFrameSize() {
    return 
    1 +  // SOF
    3 +  // HEADER
    mPayloadLength + 
    1; // CRC
}

uint8_t Deserializer::calculateCrc() {
    uint8_t crc = 0xFF;

    //-1 to exclude using CRC byte to calculate CRC
    for (size_t i = 0; i < this->getRequiredFrameSize() - 1; ++i)
    {
        crc ^= mFrameData[i];

        for (int bit = 0; bit < 8; ++bit)
        {
            if (crc & 0x80)
                crc = (uint8_t)((crc << 1) ^ 0x1D);
            else
                crc = (uint8_t)(crc << 1);
        }
    }

    return crc ^ 0xFF;
}

void Deserializer::registerPayloadCallback(std::function<void(deserializer::MessageType, MessageFrame)> func) {
    payload_callback = func;
}

bool Deserializer::deserialize_data(uint8_t* buffer, size_t readSize) { 
    // Logger::getInstance().debug("Deserializer: Received new buffer. size: " + std::to_string(readSize));
    while (mBufIdx < readSize) {
        switch (mState) {
            case deserializer::NO_SOF:
            if (buffer[mBufIdx] != 0x40) {
                // Logger::getInstance().debug("Deserializer: SOF not found. ignoring");
            } else {
                // Logger::getInstance().info("Deserializer: SOF found. Starting frame");
                mState = deserializer::READ_HEADER;
            }
            break;
            case deserializer::READ_HEADER:
            if (mHeaderVersion == -1) {
                // Logger::getInstance().debug("Deserializer: Reading the header");
                mHeaderVersion = buffer[mBufIdx];
                // Logger::getInstance().info("Header Version" + std::to_string(mHeaderVersion));
            } else if (mPayloadLength == -1) {
                mPayloadLength = buffer[mBufIdx];
                // Logger::getInstance().info("Payload length Version" + std::to_string(mPayloadLength));
            } else {
                mOptions = buffer[mBufIdx];
                // Logger::getInstance().info("Reading options" + std::to_string(mOptions));
                // Logger::getInstance().debug("Deserializer: Done reading header.");
                mFrameData = (uint8_t*)calloc(this->getRequiredFrameSize(), sizeof(uint8_t));
                if (mFrameData == NULL) {
                    this->resetFrameState();
                    return false;
                }
                mFrameData[mFrameIdx++] = SOF;
                mFrameData[mFrameIdx++] = mHeaderVersion;
                mFrameData[mFrameIdx++] = mPayloadLength;
                mFrameData[mFrameIdx++] = mOptions;
                mState = deserializer::READ_PAYLOAD;
            }
            break;
            case deserializer::READ_PAYLOAD:
            // Logger::getInstance().debug("Deserializer: Reading payload.");
            if (mPayloadRead < mPayloadLength) {
                mFrameData[mFrameIdx++] = buffer[mBufIdx];
                mPayloadRead++;
            }
            if (mPayloadRead == mPayloadLength) {
                mPayloadRead = 0;
                mState = deserializer::CALCULATE_CRC;
            }
            break;
            case deserializer::CALCULATE_CRC:
            // Logger::getInstance().info("Deserializer: Calculating CRC.");
            mFrameData[mFrameIdx] = buffer[mBufIdx];
            uint8_t calculatedCrc = this->calculateCrc();
            if (mFrameData[mFrameIdx] != calculatedCrc) {
                Logger::getInstance().error("Deserializer: CRC is invalid");
                std::string log = "calculated: " + std::to_string(calculatedCrc) + " received: " + std::to_string(mFrameData[mFrameIdx]);
                Logger::getInstance().error(log);
            } else {
                MessageFrame frame = {
					.sof = mFrameData[0],
					.version = mFrameData[1],
					.payload_length = mFrameData[2],
					.options = mFrameData[3],
					.crc = mFrameData[mFrameIdx]
				};

				if (!parse_payload(mFrameData + 4, frame.payload_length,
						&frame)) {
					Logger::getInstance().error("Invalid payload");
				} else {
                    if (payload_callback != NULL) {
                        if (frame.type == deserializer::DISTANCE_DATA) {
                            payload_callback(deserializer::DISTANCE_DATA, frame);
                        } else if (frame.type == deserializer::STATUS_DATA) {
                            payload_callback(deserializer::STATUS_DATA, frame);
                        }
                    } else {
                        Logger::getInstance().error("Deserializer: Payload callback is NULL");
                    }
				}
            }
            resetFrameState();
            break;
        }
        mBufIdx++;
    }
    mBufIdx = 0;
    return true;
}

void Deserializer::resetFrameState() {
    Logger::getInstance().error("Deserializer: Resetting state machine.");
    mState = deserializer::NO_SOF;
    mHeaderVersion = -1;
    mPayloadLength = -1;
    mOptions = -1;
    mFrameIdx = 0;
    mPayloadRead = 0;
    if (mFrameData != NULL) {
        free(mFrameData);
        mFrameData = NULL;
    }
}