#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <Mathematics.h>
#include <cstring>

namespace Firework
{
    namespace GL
    {
        class Renderer;
        class LitObjectHandle;

        struct __firework_gl_api RenderTransform
        {
            Mathematics::Matrix4x4 tf;
            
            void translate(Mathematics::Vector3 vec);
            void rotate(Mathematics::Vector3 vec);
            void rotate(Mathematics::Quaternion rot);
            void scale(Mathematics::Vector3 vec);

            friend class Firework::GL::Renderer;
            friend class Firework::GL::LitObjectHandle;
        private:
            float normalMatrix[3][3];

            void regenerateNormalMatrix();
        };
    }
}