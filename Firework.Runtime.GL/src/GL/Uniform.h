#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <cstddef>

namespace Firework::GL
{
    class Renderer;

    enum class UniformType
    {
        Vec4 = bgfx::UniformType::Vec4,
        Mat3 = bgfx::UniformType::Mat3,
        Mat4 = bgfx::UniformType::Mat4
    };

    struct __firework_gl_api UniformHandle final
    {
        static UniformHandle create(const char* name, UniformType type);
        static UniformHandle createArray(const char* name, UniformType type, uint16_t count);
        void destroy();

        operator bool () const;
        inline bool operator==(std::nullptr_t)
        {
            return !(*this);
        }

        friend class Firework::GL::Renderer;
    private:
        bgfx::UniformHandle internalHandle { bgfx::kInvalidHandle };
    };
}