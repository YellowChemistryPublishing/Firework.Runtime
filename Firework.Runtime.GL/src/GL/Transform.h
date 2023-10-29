#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <Mathematics.h>
#include <cstring>

namespace Firework
{
    namespace GL
    {
        class Renderer;
        class StaticLitObjectHandle;

        struct __firework_gl_api RenderTransform
        {
            void translate(Mathematics::Vector3 vec);
            void rotate(Mathematics::Vector3 vec);
            void rotate(Mathematics::Quaternion rot);
            void scale(Mathematics::Vector3 vec);

            friend class Firework::GL::Renderer;
            friend class Firework::GL::StaticLitObjectHandle;
        private:
            Mathematics::Matrix4x4 tf;
            float normalMatrix[3][3];

            void regenerateNormalMatrix();
        };
    }
}