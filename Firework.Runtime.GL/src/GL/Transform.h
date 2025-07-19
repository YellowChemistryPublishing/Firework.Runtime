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

        void translate(sysm::vector3 vec)
        {
            this->tf = sysm::matrix4x4::translate(vec) * this->tf;
        }
        void rotate(sysm::quaternion rot)
        {
            this->tf = sysm::matrix4x4::rotate(rot) * this->tf;
        }
        void scale(sysm::vector3 vec)
        {
            this->tf = sysm::matrix4x4::scale(vec) * this->tf;
        }

        friend class Firework::GL::Renderer;
        friend class Firework::GL::LitObjectHandle;
    };
} // namespace Firework::GL