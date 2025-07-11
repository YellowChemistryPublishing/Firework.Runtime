#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

namespace Firework
{
    namespace Internal
    {
        class CoreEngine;
    }

    class _fw_core_api Time final
    {
        static float frameDeltaTime;
    public:
        static float timeScale;

        Time() = delete;

        /// @brief Retrive the frame delta time.
        /// @return The time since the last frame in seconds.
        inline static float deltaTime()
        {
            return Time::frameDeltaTime;
        }

        friend class Firework::Internal::CoreEngine;
    };
} // namespace Firework