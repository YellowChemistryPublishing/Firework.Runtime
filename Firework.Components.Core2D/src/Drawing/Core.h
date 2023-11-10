#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <cstdint>

namespace Firework
{
    struct Color
    {
        union
        {
            struct
            {
                uint8_t r;
                uint8_t g;
                uint8_t b;
                uint8_t a;
            };
            uint8_t data[4] {0};
        };

        constexpr Color() noexcept = default;
        constexpr Color(const uint8_t (&color)[3]) noexcept :
        r(color[0]), g(color[1]), b(color[2]), a(255)
        { }
        constexpr Color(const uint8_t (&color)[4]) noexcept :
        r(color[0]), g(color[1]), b(color[2]), a(color[3])
        { }
        constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept :
        r(r), g(g), b(b), a(a)
        { }
        
        constexpr bool operator==(const Color& rhs) const noexcept
        {
            return this->r == rhs.r && this->g == rhs.g && this->b == rhs.b && this->a == rhs.a;
        }
    };
}