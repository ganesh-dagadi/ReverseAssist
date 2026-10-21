#include "Queue.hpp"

int Queue::push(const SensorData& message) {
    {
        std::lock_guard<std::mutex> lock(mLock);
        messages_.push(message);
    }
    mCv.notify_one();
    return 0;
}

SensorData Queue::pop() {
    std::unique_lock<std::mutex> lock(mLock);
    mCv.wait(lock, [this]{
        return !messages_.empty();
    });
    SensorData message = messages_.front();
    messages_.pop();
    return message;
}

std::optional<SensorData> Queue::pop(const std::atomic<bool>& stopRequested) {
    std::unique_lock<std::mutex> lock(mLock);
    mCv.wait(lock, [this, &stopRequested]{
        return !messages_.empty() || stopRequested.load();
    });
    if (messages_.empty()) {
        return std::nullopt;
    }
    SensorData message = messages_.front();
    messages_.pop();
    return message;
}

void Queue::notifyAll() {
    mCv.notify_all();
}

bool Queue::isEmpty() {
    std::lock_guard<std::mutex> lock(mLock);
    return messages_.empty();
}
