#pragma once

#include <cstddef>
#include <cstdint>

class IStreamSender {
public:
    virtual ~IStreamSender() = default;

    virtual bool initialize() = 0;
    virtual std::size_t writeBytes(const std::uint8_t* buffer, std::size_t size) = 0;
};
