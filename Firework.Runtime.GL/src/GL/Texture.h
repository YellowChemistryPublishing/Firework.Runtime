#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <cstddef>
#include <module/sys>
#include <span>

#include <GL/Common.h>

namespace Firework::GL
{
    class Renderer;

    struct _fw_gl_api TextureSampler final
    {
        TextureSampler(std::string_view name);

        _fw_gl_common_handle_interface(TextureSampler);
        _fw_gl_common_handle_swap(TextureSampler);
    private:
        bgfx::UniformHandle internalHandle { bgfx::kInvalidHandle };
    };

    struct _fw_gl_api Texture2D final
    {
        Texture2D(std::span<const byte> textureData, u16 width, u16 height, bool hasMipMaps = false, u16 layerCount = 1,
                  bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8, u64 flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE);
        Texture2D(u16 width, u16 height, bool hasMipMaps = false, u16 layerCount = 1, bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8,
                  u64 flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE);

        void updateDynamic(std::span<const byte> textureData, u16 layer, u8 mip, u16 x, u16 y, u16 width, u16 height);
        void copyTo(const Texture2D& dest, u16 dstX, u16 dstY, u16 srcX, u16 srcY, u16 width, u16 height);
        void copyTo(u16 view, const Texture2D& dest, u16 dstX, u16 dstY, u16 srcX, u16 srcY, u16 width, u16 height);

        _fw_gl_common_handle_interface(Texture2D);
        _fw_gl_common_handle_swap(Texture2D);
    private:
        bgfx::TextureHandle internalHandle { bgfx::kInvalidHandle };
    };
} // namespace Firework::GL
