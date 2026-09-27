#include "UARTStreamSender.hpp"

bool UARTStreamSender::initialize() {
    return false;
}

std::size_t UARTStreamSender::writeBytes(const std::uint8_t*, std::size_t) {
    return 0;
}
