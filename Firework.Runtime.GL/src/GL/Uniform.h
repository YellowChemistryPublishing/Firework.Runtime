#pragma once

#include "Firework.Runtime.GL.Exports.h"

_push_nowarn_clang(_clWarn_clang_zero_as_nullptr);
#include <bgfx/bgfx.h>
#include <cstddef>
#include <module/sys>
#include <utility>
_pop_nowarn_clang();

#include <GL/Common.h>

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
        Uniform(std::string_view name, UniformType type, u16 count = 1_u16);

        _fw_gl_common_handle_interface(Uniform);
        _fw_gl_common_handle_swap(Uniform);
    private:
        bgfx::UniformHandle internalHandle { bgfx::kInvalidHandle };
    };
} // namespace Firework::GL
