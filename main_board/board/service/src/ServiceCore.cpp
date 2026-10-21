#include "ServiceCore.hpp"
#include "Logging.hpp"

#include "UARTSerialDevice.hpp"

#include <exception>
#include <stdexcept>
#include <string>

using namespace std;

bool isTerminated = false;

ServiceCore::ServiceCore()
    : mSerialDevice(make_shared<UARTSerialDevice>(mSerialDevicePath)),
      mReceiver(mSerialDevice, mDeserializer, mReceiverQueue),
      mTransmitter(mSerialDevice, mSerializer, mTransmitterQueue),
      mLogger(Logger::getInstance()) {}

void ServiceCore::receiver_queue_consumer_main() {
    mLogger.info("ServiceCore: Receiver queue consumer thread run");
    while (const auto message = mReceiverQueue.pop(mConsumerStopSignaled)) {
        if (const auto* distance = std::get_if<MessageDistance>(&message->data)) {
            mLogger.info(
                "ServiceCore: Received distance data: sensor_id=" +
                std::to_string(distance->sensor_id) +
                ", distance=" + std::to_string(distance->distance));
        } else if (const auto* status = std::get_if<MessageStatus>(&message->data)) {
            mLogger.info(
                "ServiceCore: Received status data: sensor_id=" +
                std::to_string(status->sensor_id) +
                ", status=" + std::to_string(status->status));
        }
    }
    mLogger.info("ServiceCore: Receiver queue consumer thread stopped");
}

void ServiceCore::start_receiver_queue_consumer() {
    mLogger.info("ServiceCore: Starting receiver queue consumer");
    {
        std::lock_guard<std::mutex> lock(mConsumerThreadControlLock);
        if (mIsConsumerStarted) {
            throw std::runtime_error("Receiver queue consumer is already started");
        }
        mReceiverQueueConsumerThread =
            std::thread(&ServiceCore::receiver_queue_consumer_main, this);
        mIsConsumerStarted = true;
    }
}

void ServiceCore::stop_receiver_queue_consumer() {
    mLogger.info("ServiceCore: Stopping receiver queue consumer");
    {
        std::lock_guard<std::mutex> lock(mConsumerThreadControlLock);
        if (!mIsConsumerStarted) {
            throw std::runtime_error("Receiver queue consumer is not started");
        }
        mConsumerStopSignaled.store(true);
    }
    mReceiverQueue.notifyAll();
    mReceiverQueueConsumerThread.join();
    {
        std::lock_guard<std::mutex> lock(mConsumerThreadControlLock);
        mIsConsumerStarted = false;
        mConsumerStopSignaled.store(false);
    }
}

int ServiceCore::startServiceCore() {
    if (!mSerialDevice->setupDevice()) {
        mLogger.error("FATAL: failed to setup serial device");
        // TODO: notify HMI if needed;
        return -1;
    }
    mReceiver.init();
    start_receiver_queue_consumer();
    try {
        mReceiver.start();
    } catch (const std::exception&) {
        stop_receiver_queue_consumer();
        throw;
    }
    return 0;
}

int ServiceCore::resumeServiceCore() {
    start_receiver_queue_consumer();
    try {
        mReceiver.start();
    } catch (const std::exception&) {
        stop_receiver_queue_consumer();
        throw;
    }
    return 0;
}

void ServiceCore::stopServiceCore() {
    mLogger.info("ServiceCore: Stop signal received");
    mReceiver.stop();
    stop_receiver_queue_consumer();
    isTerminated = true;
}