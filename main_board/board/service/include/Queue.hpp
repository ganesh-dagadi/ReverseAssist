#pragma once

#include <cstddef>
#include <atomic>
#include <optional>
#include <queue>
#include <string>
#include <variant>
#include "SensorData.hpp"
#include <condition_variable>
#include <mutex>

class Queue {
private:
	std::condition_variable mCv;
	std::queue<SensorData> messages_;
	std::string queueFullStatus_ = "DROP_OLDEST";
	std::mutex mLock;


public:
	int push(const SensorData& message);
	SensorData pop();
	std::optional<SensorData> pop(const std::atomic<bool>& stopRequested);
	void notifyAll();
	bool isEmpty();
};
