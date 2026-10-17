#include "UARTSerialDevice.hpp"
#include "Logging.hpp"
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <errno.h>
#include <string>

UARTSerialDevice::UARTSerialDevice(const char* path)
    : mFd(-1), mPath(path) {}

UARTSerialDevice::UARTSerialDevice(UARTSerialDevice&& other) noexcept {
    std::lock_guard<std::mutex> lock(mLock);
    this->mPath = other.mPath;
    this->mFd = -1;
    this->mFd = other.mFd;
    other.mFd = -1;
}

UARTSerialDevice& UARTSerialDevice::operator=(UARTSerialDevice&& other) noexcept {
    std::lock_guard<std::mutex> lock(mLock);
    if (this == &other) return *this;
    if (this->mFd != -1) {
        Logger::getInstance().info("UARTSerialDevice: Transferee object has fd open. Closing");
        const int ret = close(this->mFd);
        if (ret == -1) {
            const int error = errno;
            Logger::getInstance().error(
                std::string("UARTSerialDevice: Close of transferee failed in move (errno ") +
                std::to_string(error) + ": " + std::strerror(error) + ")");
            return *this;
        }
    }
    this->mFd = other.mFd;
    other.mFd = -1;
    return *this;
}
UARTSerialDevice::~UARTSerialDevice() {
    std::lock_guard<std::mutex> lock(mLock);
    if (mFd == -1) {
        Logger::getInstance().info("UARTSerialDevice: Fd is already cleared");
        return;
    }
    const int ret = close(mFd);
    if (ret == -1) {
        const int error = errno;
        Logger::getInstance().error(
            std::string("UARTSerialDevice: Close failed in destructor (errno ") +
            std::to_string(error) + ": " + std::strerror(error) + ")");
        return;
    }
    mFd = -1;
}
bool UARTSerialDevice::setupDevice() {
    std::lock_guard<std::mutex> lock(mLock);
    if (mFd != -1) {
        Logger::getInstance().error("UARTSerial device: Setup device fail. fd is not reset");
        return false;
    }
    const auto logError = [](const char* operation, int error) {
        Logger::getInstance().error(
            std::string("UARTSerialDevice: ") + operation + " failed (errno " +
            std::to_string(error) + ": " + std::strerror(error) + ")");
    };

    const int fd = open(mPath, O_RDWR | O_NOCTTY | O_CLOEXEC);
    if (fd == -1) {
        logError("open", errno);
        return false;
    }

    termios settings{};
    if (tcgetattr(fd, &settings) == -1) {
        const int error = errno;
        close(fd);
        logError("tcgetattr", error);
        return false;
    }

    cfmakeraw(&settings);
    settings.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
    settings.c_cflag |= CS8 | CLOCAL | CREAD;
#ifdef CRTSCTS
    settings.c_cflag &= ~CRTSCTS;
#endif

    // do not perform busy waiting until atleast one byte is available
    settings.c_cc[VMIN] = 1;
    settings.c_cc[VTIME] = 0;

    if (cfsetispeed(&settings, B115200) == -1 ||
        cfsetospeed(&settings, B115200) == -1 ||
        tcsetattr(fd, TCSANOW, &settings) == -1) {
        const int error = errno;
        close(fd);
        logError("configure 115200 8N1", error);
        return false;
    }

    mFd = fd;
    return true;
}

std::size_t UARTSerialDevice::readData(std::uint8_t* buffer, std::size_t size) {
    std::size_t bytesRead = 0;
    if (mFd == -1) {
        Logger::getInstance().error("UartSerialDevice: read failed. Invalid file descriptor");
        return 0;
    }
    bytesRead = read(mFd, buffer, size);
    if (bytesRead < 0) {
        int errorNum = errno;
        Logger::getInstance().error(
            std::string("UARTSerialDevice: ")+ "read failed (errno " +
            std::to_string(errorNum) + ": " + std::strerror(errorNum) + ")");
    }
    return bytesRead;
}

std::size_t UARTSerialDevice::writeData(const std::uint8_t*, std::size_t) {
    return 0;
}
