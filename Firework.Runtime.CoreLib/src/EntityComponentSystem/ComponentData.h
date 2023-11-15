#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <Objects/Component2D.h>
#include <Objects/Component.h>

namespace Firework
{
    /// @brief Base type for all external 2D library components to derive from. That's you!
    /// @note Main thread only. Components aren't thread safe, so the engine will assume that yours aren't either.
    struct ComponentData2D : public Internal::Component2D
    {
        inline ~ComponentData2D() override = 0;
    };
    ComponentData2D::~ComponentData2D() = default;
    
    /// @brief Base type for all external 3D library components to derive from. This may also be you!
    /// @note Main thread only. Components aren't thread safe, so the engine will assume that yours aren't either.
    struct ComponentData : public Internal::Component
    {
        inline ~ComponentData() override = 0;
    };
    ComponentData::~ComponentData() = default;
}