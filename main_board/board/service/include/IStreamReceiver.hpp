#pragma once

#include <cstddef>
#include <cstdint>

class IStreamReceiver {
public:
    virtual ~IStreamReceiver() = default;

    virtual bool initialize() = 0;
    virtual std::size_t readBytes(std::uint8_t* buffer, std::size_t size) = 0;
};
