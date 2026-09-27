#pragma once

#include "Deserializer.hpp"
#include "IStreamReceiver.hpp"
#include "Queue.hpp"

class DataReceiver {
private:
	const Deserializer& mDeserializer;
	IStreamReceiver& mReceiver;
	Queue& mDataQueue;

public:
	DataReceiver(IStreamReceiver& receiver, const Deserializer& deserializer, Queue& dataQueue)
        : mReceiver(receiver), mDeserializer(deserializer), mDataQueue(dataQueue) {}
};
