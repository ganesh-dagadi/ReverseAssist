#pragma once

#include <cstddef>
#include <cstdint>

class SerialDevice {

public:
    SerialDevice() = default;
    virtual ~SerialDevice() = default;
    SerialDevice(const SerialDevice& other) = delete;
    SerialDevice& operator =(const SerialDevice& other) = delete;
    SerialDevice(SerialDevice&& other) noexcept = default;
    SerialDevice& operator =(SerialDevice&& other) noexcept = default;
    virtual bool setupDevice() = 0;
    virtual std::size_t readData(std::uint8_t* buffer, std::size_t size) = 0;
    virtual std::size_t writeData(const std::uint8_t* buffer, std::size_t size) = 0;
};
