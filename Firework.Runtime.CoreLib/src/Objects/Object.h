#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

namespace Firework::Internal
{
    /// @brief Base type of all objects that can exist within the hierachy of the runtime.
    class Object
    {
    public:
        inline virtual ~Object() = 0;
    };

    Object::~Object() = default;
}
