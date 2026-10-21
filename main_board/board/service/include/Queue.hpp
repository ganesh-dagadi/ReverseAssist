#pragma once

#include <cstddef>
#include <queue>
#include <string>
#include <variant>
#include "SensorData.hpp"
#include <mutex>

class Queue {
private:
	std::queue<SensorData> messages_;
	std::string queueFullStatus_ = "DROP_OLDEST";
	std::mutex mLock;

public:
	int push(const SensorData& message);
	SensorData pop();
	bool isEmpty();
};
