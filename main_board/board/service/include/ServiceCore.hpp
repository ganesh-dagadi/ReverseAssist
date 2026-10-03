#pragma once

#include "DataReceiver.hpp"
#include "DataSender.hpp"
#include "Queue.hpp"
#include "Logging.hpp"
#include "SerialDevice.hpp"
#include <memory>

class ServiceCore {
private:
    std::shared_ptr<SerialDevice> mSerialDevice;
    Deserializer mDeserializer;
    Queue mReceiverQueue;
    DataReceiver mReceiver;
    Serializer mSerializer;
    Queue mTransmitterQueue;
    DataSender mTransmitter;
    Logger& mLogger;

public:
    ServiceCore();
    int startServiceCore();
    int resumeServiceCore();
    void stopServiceCore();
};