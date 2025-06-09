#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <GL/Geometry.h>
#include <GL/Shader.h>
#include <Objects/Component2D.h>

namespace Firework
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;
    }

    class __firework_componentcore2d_api Mask final : public Internal::Component2D
    {
        static GL::StaticMeshHandle unitSquare;
        static GL::GeometryProgramHandle program;
        
        static uint32_t currentRenderMaskValue;

        static void renderInitialize();
        static void renderFirstPass(bgfx::ViewId, void*); // Render thread.
        void renderOffload();
        void lateRenderOffload();

        // Rendering ^
    public:
        ~Mask() override = default;

        friend struct Firework::Internal::ComponentCoreStaticInit;
    };
}