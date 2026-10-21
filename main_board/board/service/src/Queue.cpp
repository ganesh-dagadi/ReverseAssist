#include "Queue.hpp"

int Queue::push(const SensorData& message) {
    std::lock_guard<std::mutex> lock(mLock);
    messages_.push(message);
    return 0;
}

SensorData Queue::pop() {
    std::lock_guard<std::mutex> lock(mLock);
    if (messages_.empty()) {
        return {};
    }

    SensorData message = messages_.front();
    messages_.pop();
    return message;
}

bool Queue::isEmpty() {
    std::lock_guard<std::mutex> lock(mLock);
    return messages_.empty();
}
