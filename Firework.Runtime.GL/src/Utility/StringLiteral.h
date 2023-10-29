#pragma once

#include <algorithm>
#include <cstdint>

namespace Firework
{
    template <size_t Length>
    struct StringLiteral
    {
        char str[Length];

        constexpr StringLiteral(const char (&literal)[Length])
        {
            std::copy_n(str, Length, literal);
        }
    };
}