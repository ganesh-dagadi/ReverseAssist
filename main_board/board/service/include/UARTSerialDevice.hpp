#pragma once

#include "SerialDevice.hpp"

class UARTSerialDevice : public SerialDevice {
public:
    std::size_t read(std::uint8_t* buffer, std::size_t size) override;
    std::size_t write(const std::uint8_t* buffer, std::size_t size) override;
};
