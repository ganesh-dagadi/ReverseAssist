#pragma once
#include <variant>

typedef struct MessageDistance {
	unsigned int sensor_id;
	float distance;
} MessageDistance;

typedef struct MessageStatus {
	unsigned int sensor_id;
	int status;
} MessageStatus;

class SensorData {
	public:
	std::variant<MessageDistance, MessageStatus> data;
};