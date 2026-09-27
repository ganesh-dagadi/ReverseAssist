#include "ServiceCore.hpp"
#include "iostream"

#include "UARTStreamReceiver.hpp"
#include "UARTStreamSender.hpp"

using namespace std;

bool isTerminated = false;

ServiceCore::ServiceCore()
        :mLogger(Logger::getInstance()),
        mStreamReceiver(make_unique<UARTStreamReceiver>()),
        mReceiver(DataReceiver(*mStreamReceiver, mDeserializer, mReceiverQueue)),
        mStreamSender(make_unique<UARTStreamSender>()),
        mTransmitter(DataTransmiter(*mStreamSender, mSerializer, mTransmitterQueue))
        {}  

int ServiceCore::startServiceCore() {
    while (!isTerminated) {
        cout << "Hello \n";
    }
    return 0;
}

void ServiceCore::stopServiceCore() {
    cout << "Stop service received inside Service core \n";
    isTerminated = true;
}