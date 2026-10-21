#include "DataReceiver.hpp"
#include "Deserializer.hpp"
#include "SensorData.hpp"
#include "Logging.hpp"
#include <utility>
#include <stdexcept>

void DataReceiver::payload_callback(deserializer::MessageType type, MessageFrame payload) {
    
}

void DataReceiver::data_receiver_main() {
    Logger::getInstance().info("DataReceiver: thread run");
    uint8_t buf[16];
    for(;;) {
        {
            std::lock_guard<std::mutex> lock(mThreadRunControlLock);
            if (mIsStopSignaled) {
                Logger::getInstance().info("Data Receiver: Thread signalled to stop. Stopping");
                return;
            }
        }
        size_t bytesRead = mSerialDevice->readData(buf, 16);
        if (bytesRead != 0) {
            mDeserializer.deserialize_data(buf, bytesRead);
        }
    }
}

void DataReceiver::init() {
    Logger::getInstance().info("Data Receiver: init");
    mDeserializer.registerPayloadCallback([this](deserializer::MessageType type, MessageFrame payload) {
        SensorData message;
        if (deserializer::DISTANCE_DATA == type) {
            Logger::getInstance().debug("Received distancedata");
            message.data = MessageDistance{payload.data.distance.sensor_id, payload.data.distance.distance};
        } else if (deserializer::STATUS_DATA == type) {
            Logger::getInstance().debug("Received status data");
            message.data = MessageStatus{payload.data.status.sensor_id, payload.data.status.status};
        } else {
            Logger::getInstance().error("Data receiver: invalid message type");
        }
        mDataQueue.push(std::move(message));
    });
}
void DataReceiver::start() {
    Logger::getInstance().info("DataReceiver: Starting data receiver");
    {
        std::lock_guard<std::mutex> lock(mThreadRunControlLock);
        if (mIsStarted) {
            throw std::runtime_error("Data receiver is already started");
        }
        Logger::getInstance().info("DataReceiver: Starting data receiver thread");
        mReceiverThread = std::thread(&DataReceiver::data_receiver_main, this);
        mIsStarted = true;
    }
    Logger::getInstance().info("DataReceiver: start done");
}

void DataReceiver::stop() {
    Logger::getInstance().info("DataReceiver: Stopping data receiver");
    {
        std::lock_guard<std::mutex> lock(mThreadRunControlLock);
        if (!mIsStarted) {
            throw std::runtime_error("Data receiver is not started");
        }
        Logger::getInstance().info("DataReceiver: Stopping data receiver thread");
        mIsStopSignaled = true;
    }
    mReceiverThread.join();
    {
        std::lock_guard<std::mutex> lock(mThreadRunControlLock);
        mIsStarted = false;
        mIsStopSignaled = false;
    }
}