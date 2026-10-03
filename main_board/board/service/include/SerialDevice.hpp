#pragma once

#include <cstddef>
#include <cstdint>

class SerialDevice {
public:
    virtual ~SerialDevice() = default;

    virtual std::size_t read(std::uint8_t* buffer, std::size_t size) = 0;
    virtual std::size_t write(const std::uint8_t* buffer, std::size_t size) = 0;
};
