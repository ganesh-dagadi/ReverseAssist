#include "Queue.hpp"

int Queue::push(const Message& message) {
    messages_.push(message);
    return 0;
}

Message Queue::pop() {
    if (messages_.empty()) {
        return {};
    }

    Message message = messages_.front();
    messages_.pop();
    return message;
}

bool Queue::isEmpty() const {
    return messages_.empty();
}

const std::string& Queue::fullStatus() const {
    return queueFullStatus_;
}
