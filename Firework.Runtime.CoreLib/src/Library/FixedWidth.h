#pragma once

#include <cstdint>

namespace Firework
{
    template <size_t Width>
    using FixedWidthUInt =
    std::conditional_t<Width == 1, uint8_t,
    std::conditional_t<Width == 2, uint16_t,
    std::conditional_t<Width == 4, uint32_t,
    std::conditional_t<Width == 8, uint64_t,
    void>>>>;
    
    template <size_t Width>
    using FixedWidthInt =
    std::conditional_t<Width == 1, int8_t,
    std::conditional_t<Width == 2, int16_t,
    std::conditional_t<Width == 4, int32_t,
    std::conditional_t<Width == 8, int64_t,
    void>>>>;
}