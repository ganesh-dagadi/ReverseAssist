#pragma once

#include "DataReceiver.hpp"
#include "DataTransmiter.hpp"
#include "Queue.hpp"
#include "Logging.hpp"
#include "memory"

class ServiceCore {
private:
    std::unique_ptr<IStreamReceiver> mStreamReceiver;
    Deserializer mDeserializer;
    Queue mReceiverQueue;
    DataReceiver mReceiver;
    std::unique_ptr<IStreamSender> mStreamSender;
    Serializer mSerializer;
    Queue mTransmitterQueue;
    DataTransmiter mTransmitter;
    Logger& mLogger;

public:
    ServiceCore();
    int startServiceCore();
    void stopServiceCore();
};