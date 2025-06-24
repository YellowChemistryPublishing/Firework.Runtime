#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <module/sys.Mathematics>

#include <Library/Property.h>

namespace Firework
{
    namespace Internal
    {
        class CoreEngine;
    }

    class Cursor;

    class __firework_corelib_api Window final
    {
        static int width;
        static int height;
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
        inline static int pixelWidth()
        {
            return Window::width;
        }
        /// @brief Retrieve the height of the window.
        /// @return Height of the window in pixels.
        /// @note Main thread only.
        inline static int pixelHeight()
        {
            return Window::height;
        }
        /// @brief Set the resolution of the window.
        /// @param size Dimensions of the window in pixels.
        /// @note Main thread only.
        static void setResolution(sysm::vector2i32 resolution);

        friend class Firework::Internal::CoreEngine;
        friend class Firework::Cursor;
    };

    class __firework_corelib_api Screen final
    {
        static int width;
        static int height;
        static int screenRefreshRate;
    public:
        /// @brief Retrieve the width of the primary display.
        /// @return Width of the display in pixels.
        /// @note Main thread only.
        inline static int pixelWidth()
        {
            return Screen::width;
        }
        /// @brief Retrieve the height of the primary display.
        /// @return Height of the display in pixels.
        /// @note Main thread only.
        inline static int pixelHeight()
        {
            return Screen::height;
        }
        /// @brief Retrieve the refresh rate of the primary dislay.
        /// @return Refresh rate of the display in Hz.
        /// @note Main thread only.
        inline static int refreshRate()
        {
            return Screen::screenRefreshRate;
        }

        friend class Firework::Internal::CoreEngine;
    };
} // namespace Firework
