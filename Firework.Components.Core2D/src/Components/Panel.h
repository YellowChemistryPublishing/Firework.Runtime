#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <Drawing/Core.h>
#include <GL/Geometry.h>
#include <GL/Renderer.h>
#include <GL/Transform.h>
#include <Objects/Component2D.h>
#include <Library/Property.h>

namespace Firework
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;
    }

    class __firework_componentcore2d_api Panel final : public Internal::Component2D
    {
        static GL::StaticMeshHandle unitSquare;
        static GL::GeometryProgramHandle program;

        static void renderInitialize();
        void renderOffload();

        // Rendering ^ / v Data

        Color _color { 0, 0, 0, 255 };
    public:
        const Property<const Color&, const Color&> color
        {{
            [this]() -> const Color& { return this->_color; },
            [this](const Color& value) { this->_color = value; }
        }};

        ~Panel() override = default;

        friend struct Firework::Internal::ComponentCoreStaticInit;
    };
}