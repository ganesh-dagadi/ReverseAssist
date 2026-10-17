#include "ServiceCore.hpp"
#include "Logging.hpp"

#include "UARTSerialDevice.hpp"

using namespace std;

bool isTerminated = false;

ServiceCore::ServiceCore()
    : mSerialDevice(make_shared<UARTSerialDevice>(mSerialDevicePath)),
      mReceiver(mSerialDevice, mDeserializer, mReceiverQueue),
      mTransmitter(mSerialDevice, mSerializer, mTransmitterQueue),
      mLogger(Logger::getInstance()) {}

int ServiceCore::startServiceCore() {
    if (!mSerialDevice->setupDevice()) {
        mLogger.error("FATAL: failed to setup serial device");
        // TODO: notify HMI if needed;
        return -1;
    }
    mReceiver.start();
    return 0;
}

int ServiceCore::resumeServiceCore() {
    mReceiver.start();
    return 0;
}

void ServiceCore::stopServiceCore() {
    mLogger.info("ServiceCore: Stop signal received");
    mReceiver.stop();
    isTerminated = true;
}