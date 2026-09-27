#pragma once

#include <cstddef>
#include <queue>
#include <string>

class Message {

};

class Queue {
private:
	std::queue<Message> messages_;
	std::string queueFullStatus_ = "DROP_OLDEST";

public:
	int push(const Message& message);
	Message pop();
	bool isEmpty() const;
	const std::string& fullStatus() const;
};
