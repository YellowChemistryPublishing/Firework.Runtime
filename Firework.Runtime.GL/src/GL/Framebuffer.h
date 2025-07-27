#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <cstddef>
#include <module/sys>
#include <utility>

#include <GL/Common.h>
#include <GL/TextureFormat.h>

namespace Firework::GL
{
    class Renderer;

    struct _fw_gl_api Framebuffer final
    {
        Framebuffer(BackbufferRatio ratio, TextureFormat format = TextureFormat::Count, u64 flags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
        Framebuffer(std::span<const struct Texture2D> textures);

        _fw_gl_common_handle_interface(Framebuffer);
        _fw_gl_common_handle_swap(Framebuffer);
    private:
        bgfx::FrameBufferHandle internalHandle { bgfx::kInvalidHandle };
    };
} // namespace Firework::GL
