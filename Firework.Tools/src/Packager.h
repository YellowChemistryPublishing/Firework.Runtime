#pragma once

#include <cstdint>
#include <filesystem>
#include <string_view>

namespace Firework::PackageSystem
{
    class Packager final
    {
    public:
        static enum class Endianness : int_fast8_t {
            Big = 0,
            Little = 1
        } endianness;
        
        static void packageFolder(std::filesystem::path folder, std::filesystem::path outFile);
    };

    constexpr auto toEndianness = [](auto intType, Packager::Endianness from, Packager::Endianness to) -> decltype(intType)
    {
        if (from == to)
            return intType;
        else
        {
            decltype(intType) _int = 0;
            for (size_t i = 0; i < sizeof(decltype(intType)); i++)
                reinterpret_cast<int8_t*>(&_int)[i] = reinterpret_cast<int8_t*>(&intType)[sizeof(decltype(intType)) - 1 - i];
            return _int;
        }
    };
}