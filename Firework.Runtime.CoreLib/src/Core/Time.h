#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

namespace Firework
{
    namespace Internal
    {
        class CoreEngine;
    }

    class __firework_corelib_api Time final
    {
        static float frameDeltaTime;
    public:
        static float timeScale;
        
        inline static float deltaTime()
        {
            return Time::frameDeltaTime;
        }

        friend class Firework::Internal::CoreEngine;
    };
}