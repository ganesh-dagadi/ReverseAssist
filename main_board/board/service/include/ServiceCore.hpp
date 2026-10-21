#pragma once

#include "DataReceiver.hpp"
#include "DataSender.hpp"
#include "Queue.hpp"
#include "Logging.hpp"
#include "SerialDevice.hpp"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

class ServiceCore {
private:
    const char* mSerialDevicePath = "/dev/ttyACM1";
    std::shared_ptr<SerialDevice> mSerialDevice;
    Deserializer mDeserializer;
    std::condition_variable receiverCv;
    std::condition_variable senderCv;
    Queue mReceiverQueue;
    DataReceiver mReceiver;
    Serializer mSerializer;
    Queue mTransmitterQueue;
    DataSender mTransmitter;
    Logger& mLogger;
    std::mutex mConsumerThreadControlLock;
    std::thread mReceiverQueueConsumerThread;
    std::atomic<bool> mConsumerStopSignaled{false};
    bool mIsConsumerStarted = false;

    void receiver_queue_consumer_main();
    void start_receiver_queue_consumer();
    void stop_receiver_queue_consumer();

public:
    ServiceCore();
    int startServiceCore();
    int resumeServiceCore();
    void stopServiceCore();
};