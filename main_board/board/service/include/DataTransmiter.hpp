#pragma once

#include "IStreamSender.hpp"
#include "Queue.hpp"
#include "Serializer.hpp"

class DataTransmiter {
private:
	const Serializer& mSerializer;
	IStreamSender& mSender;
	Queue& mDataQueue;

public:
	DataTransmiter(IStreamSender& sender, const Serializer& serializer, Queue& dataQueue)
		: mSerializer(serializer), mSender(sender), mDataQueue(dataQueue) {}
};
