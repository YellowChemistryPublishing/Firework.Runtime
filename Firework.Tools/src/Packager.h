#pragma once

#include <bit>
#include <cstdint>
#include <filesystem>
#include <string_view>

namespace Firework::PackageSystem
{
    class Packager final
    {
    public:
        [[nodiscard]] static bool packageFolder(std::filesystem::path folder, std::filesystem::path outFile);

        template <std::integral T>
        constexpr static T toEndianness(T intType, std::endian from, std::endian to)
        {
            if (from == to)
                return intType;
            else
                return std::byteswap(intType);
        };
    };
} // namespace Firework::PackageSystem