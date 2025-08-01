#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <glm/vec2.hpp>
#include <module/sys>

#include <Library/Property.h>

namespace Firework::Internal
{
    class CoreEngine;
}

_push_nowarn_msvc(_clWarn_msvc_export_interface);
namespace Firework
{
    class Cursor;

    class _fw_core_api Window final
    {
        static i32 width;
        static i32 height;
        static bool resizing;
    public:
        /// @brief Retrieve whether the window is resizing this frame.
        /// @return Whether the window is resizing.
        /// @note Main thread only.
        inline static bool isResizing()
        {
            return Window::resizing;
        }
        /// @brief Retrieve the width of the window.
        /// @return Width of the window in pixels.
        /// @note Main thread only.
        inline static i32 pixelWidth()
        {
            return Window::width;
        }
        /// @brief Retrieve the height of the window.
        /// @return Height of the window in pixels.
        /// @note Main thread only.
        inline static i32 pixelHeight()
        {
            return Window::height;
        }
        /// @brief Set the resolution of the window.
        /// @param size Dimensions of the window in pixels.
        /// @note Main thread only.
        static void setResolution(glm::i32vec2 resolution);

        friend class Firework::Internal::CoreEngine;
        friend class Firework::Cursor;
    };

    class _fw_core_api Screen final
    {
        static i32 width;
        static i32 height;
        static i32 screenRefreshRate;
    public:
        /// @brief Retrieve the width of the primary display.
        /// @return Width of the display in pixels.
        /// @note Main thread only.
        inline static i32 pixelWidth()
        {
            return Screen::width;
        }
        /// @brief Retrieve the height of the primary display.
        /// @return Height of the display in pixels.
        /// @note Main thread only.
        inline static i32 pixelHeight()
        {
            return Screen::height;
        }
        /// @brief Retrieve the refresh rate of the primary dislay.
        /// @return Refresh rate of the display in Hz.
        /// @note Main thread only.
        inline static i32 refreshRate()
        {
            return Screen::screenRefreshRate;
        }

        friend class Firework::Internal::CoreEngine;
    };
} // namespace Firework
_pop_nowarn_msvc();
