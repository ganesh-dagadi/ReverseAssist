#include "DataReceiver.hpp"
#include "Logging.hpp"
#include <stdexcept>

void DataReceiver::data_receiver_main() {
    Logger::getInstance().info("DataReceiver: thread run");
    for(;;) {
        {
            std::lock_guard<std::mutex> lock(mThreadRunControlLock);
            if (mIsStopSignaled) {
                Logger::getInstance().info("Data Receiver: Thread signalled to stop. Stopping");
                return;
            }
        }
        
    }
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