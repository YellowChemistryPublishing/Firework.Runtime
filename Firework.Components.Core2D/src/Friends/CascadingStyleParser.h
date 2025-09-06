#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <map>
#include <module/sys>

namespace Firework
{
    class VectorParser;

    class _fw_cc2d_api CascadingStyleParser final
    {
        static void ignoreWhitespace(const char*& it, const char* end);
    public:
        CascadingStyleParser() = delete;

        [[nodiscard]] static bool parseBlock(std::string_view block, std::map<std::string_view, std::string_view>& out);

        friend class Firework::VectorParser;
    };
} // namespace Firework
