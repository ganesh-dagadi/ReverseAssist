#include "DataReceiver.hpp"
#include "Deserializer.hpp"
#include "Logging.hpp"
#include <stdexcept>

void payload_detected(deserializer::MessageType type, void* payload) {

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
    mDeserializer.registerPayloadCallback([](deserializer::MessageType type, void* payload){

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