#pragma once

#include "Deserializer.hpp"
#include "SerialDevice.hpp"
#include <memory>
#include "Queue.hpp"
#include <mutex>
#include <thread>

class DataReceiver {
private:
	const Deserializer& mDeserializer;
	std::shared_ptr<SerialDevice> mSerialDevice;
	Queue& mDataQueue;
	bool mIsStarted = false;
	bool mIsStopSignaled = false;
	// always acquire locks in declaration order
	std::mutex mLock;
	std::mutex mThreadRunControlLock;
	std::thread mReceiverThread;

	void data_receiver_main();

public:
	DataReceiver(const std::shared_ptr<SerialDevice>& serialDevice, const Deserializer& deserializer, Queue& dataQueue)
		: mDeserializer(deserializer), mSerialDevice(serialDevice), mDataQueue(dataQueue) {}
	
	/*
		desc: start the Data Receiver, and receive messages in data queue
		param: none
		return: none
		throws: IllegalStateException
	*/
	void start();

	/*
		stop the Data Receiver,
		throws IllegalStateException
	*/
	void stop();
};
