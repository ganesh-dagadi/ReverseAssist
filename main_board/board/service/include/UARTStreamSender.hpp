#pragma once

#include "IStreamSender.hpp"

class UARTStreamSender : public IStreamSender {
public:
    bool initialize() override;
    std::size_t writeBytes(const std::uint8_t* buffer, std::size_t size) override;
};
