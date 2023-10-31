#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <Objects/Component2D.h>
#include <Objects/Component.h>

namespace Firework
{
    struct ComponentData2D : public Internal::Component2D
    {
        inline virtual ~ComponentData2D() override = 0;
    };
    inline ComponentData2D::~ComponentData2D() = default;
    
    struct ComponentData : public Internal::Component
    {
        inline virtual ~ComponentData() override = 0;
    };
    inline ComponentData::~ComponentData() = default;
}