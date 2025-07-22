#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <cstddef>
#include <module/sys>
#include <utility>

namespace Firework::GL
{
    class Renderer;

    enum class UniformType
    {
        Vec4 = bgfx::UniformType::Vec4,
        Mat3 = bgfx::UniformType::Mat3,
        Mat4 = bgfx::UniformType::Mat4
    };

    struct _fw_gl_api Uniform final
    {
        Uniform(std::nullptr_t) { };
        Uniform(std::string_view name, UniformType type, u16 count = 1_u16);
        Uniform(const Uniform&) = delete;
        Uniform(Uniform&& other)
        {
            swap(*this, other);
        }
        ~Uniform();

        operator bool() const noexcept
        {
            return bgfx::isValid(this->internalHandle);
        }

        friend void swap(Uniform& a, Uniform& b) noexcept
        {
            using std::swap;

            swap(a.internalHandle, b.internalHandle);
        }

        friend class Firework::GL::Renderer;
    private:
        bgfx::UniformHandle internalHandle { bgfx::kInvalidHandle };
    };
} // namespace Firework::GL
