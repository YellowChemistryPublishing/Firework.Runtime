#pragma once

#include <initializer_list>
#include <utility>

namespace Firework::Internal
{
    constexpr std::initializer_list<std::pair<float, float>> solidIntersectionOffsets { { -1.0f, -1.0f }, { -1.0f, 1.0f }, { 1.0f, -1.0f }, { 1.0f, 1.0f } };

    constexpr std::initializer_list<std::pair<float, float>> multisampleOffsets8 { { -0.5f + 1.0f / 8.0f, -0.5f + 7.0f / 8.0f }, // | . o . . . . . .
                                                                                   { -0.5f + 6.0f / 8.0f, -0.5f + 6.0f / 8.0f }, // | . . . . . . o .
                                                                                   { -0.5f + 3.0f / 8.0f, -0.5f + 5.0f / 8.0f }, // | . . . o . . . .
                                                                                   { -0.5f + 5.0f / 8.0f, 0.0f },                // | . . . . . o . .
                                                                                   { -0.5f + 2.0f / 8.0f, -0.5f + 3.0f / 8.0f }, // | . . o . . . . .
                                                                                   { -0.5f + 4.0f / 8.0f, -0.5f + 2.0f / 8.0f }, // | . . . . o . . .
                                                                                   { -0.5f + 7.0f / 8.0f, -0.5f + 1.0f / 8.0f }, // | . . . . . . . o
                                                                                   { -0.5f + 0.0f / 8.0f, -0.5f } };             // | o . . . . . . .
    constexpr std::initializer_list<std::pair<float, float>> multisampleOffsets4 {
        { -0.5f + 2.0f / 4.0f, -0.5f + 3.0f / 4.0f }, // | . . o .
        { -0.5f, -0.5f + 2.0f / 4.0f },               // | o . . .
        { -0.5f + 3.0f / 4.0f, -0.5f + 1.0f / 4.0f }, // | . . . o
        { -0.5f + 1.0f / 4.0f, -0.5f },               // | . o . .
    };

    constexpr inline const std::initializer_list<std::pair<float, float>>& multisampleOffsets = multisampleOffsets8;
} // namespace Firework::Internal
