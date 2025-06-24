#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <cstring>
#include <module/sys.Mathematics>

namespace Firework::GL
{
    class Renderer;
    class LitObjectHandle;

    struct __firework_gl_api RenderTransform
    {
        sysm::matrix4x4 tf;

        void translate(sysm::vector3 vec);
        void rotate(sysm::quaternion rot);
        void scale(sysm::vector3 vec);

        friend class Firework::GL::Renderer;
        friend class Firework::GL::LitObjectHandle;
    private:
        float normalMatrix[3][3];

        void regenerateNormalMatrix();
    };
} // namespace Firework::GL