#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <cstdint>
#include <module/sys>

namespace Firework
{
    struct _packed Color
    {
        static const Color unknown;

        _push_nowarn_gcc(_clWarn_gcc_pedantic);
        _push_nowarn_clang(_clWarn_clang_anon_union_struct);
        _push_nowarn_clang(_clWarn_clang_nameless_struct_union);
        _push_nowarn_msvc(_clWarn_msvc_nameless_struct_union);
        union _packed
        {
            struct
            {
                uint8_t r;
                uint8_t g;
                uint8_t b;
                uint8_t a;
            };
            uint8_t data[4] {};
        };
        _pop_nowarn_msvc();
        _pop_nowarn_clang();
        _pop_nowarn_clang();
        _pop_nowarn_gcc();

        constexpr Color() noexcept = default;
        constexpr Color(const uint8_t (&color)[3]) noexcept : r(color[0]), g(color[1]), b(color[2]), a(255)
        { }
        constexpr Color(const uint8_t (&color)[4]) noexcept : r(color[0]), g(color[1]), b(color[2]), a(color[3])
        { }
        constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept : r(r), g(g), b(b), a(a)
        { }

        constexpr bool operator==(const Color& rhs) const noexcept
        {
            return this->r == rhs.r && this->g == rhs.g && this->b == rhs.b && this->a == rhs.a;
        }
    };

    constexpr inline const Color Color::unknown(255, 165, 0);
} // namespace Firework
