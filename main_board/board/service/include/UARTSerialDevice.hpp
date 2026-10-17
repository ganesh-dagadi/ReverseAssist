#pragma once

#include "SerialDevice.hpp"
#include <mutex>

class UARTSerialDevice : public SerialDevice {
    private:
        int mFd;
        const char* mPath;
        std::mutex mLock;
    public:
        UARTSerialDevice() = default;
        UARTSerialDevice(const char* path);
        ~UARTSerialDevice();
        UARTSerialDevice(const UARTSerialDevice& other) = delete;
        UARTSerialDevice& operator=(const UARTSerialDevice& other) = delete;
        UARTSerialDevice(UARTSerialDevice&& other) noexcept;
        UARTSerialDevice& operator= (UARTSerialDevice&& other) noexcept;
        bool setupDevice();
        std::size_t readData(std::uint8_t* buffer, std::size_t size) override;
        std::size_t writeData(const std::uint8_t* buffer, std::size_t size) override;
};
