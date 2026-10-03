#include "ServiceCore.hpp"
#include "Logging.hpp"

#include "UARTSerialDevice.hpp"

using namespace std;

bool isTerminated = false;

ServiceCore::ServiceCore()
    : mSerialDevice(make_shared<UARTSerialDevice>()),
      mReceiver(mSerialDevice, mDeserializer, mReceiverQueue),
      mTransmitter(mSerialDevice, mSerializer, mTransmitterQueue),
      mLogger(Logger::getInstance()) {}

int ServiceCore::startServiceCore() {
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