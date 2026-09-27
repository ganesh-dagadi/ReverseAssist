#pragma once

#include "IStreamReceiver.hpp"

class UARTStreamReceiver : public IStreamReceiver {
public:
    bool initialize() override;
    std::size_t readBytes(std::uint8_t* buffer, std::size_t size) override;
};
