#pragma once

#include "Queue.hpp"
#include "SerialDevice.hpp"
#include "Serializer.hpp"
#include <memory>

class DataSender {
private:
    const Serializer& mSerializer;
    std::shared_ptr<SerialDevice> mSerialDevice;
    Queue& mDataQueue;

public:
    DataSender(const std::shared_ptr<SerialDevice>& serialDevice, const Serializer& serializer, Queue& dataQueue)
        : mSerializer(serializer), mSerialDevice(serialDevice), mDataQueue(dataQueue) {}
};
