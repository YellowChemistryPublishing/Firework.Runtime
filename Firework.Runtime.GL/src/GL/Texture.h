#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <cstddef>

namespace Firework
{
    namespace GL
    {
        class Renderer;

        struct __firework_gl_api TextureSamplerHandle final
        {
            static TextureSamplerHandle create(const char* name);
            void destroy();

            inline TextureSamplerHandle([[maybe_unused]] std::nullptr_t _) noexcept
            { }
            inline TextureSamplerHandle() noexcept = default;

            inline operator bool () const noexcept
            {
                return bgfx::isValid(this->internalHandle);
            }
            inline bool operator==(std::nullptr_t)
            {
                return !bgfx::isValid(this->internalHandle);
            }
            inline bool operator==(const TextureSamplerHandle& other) const noexcept
            {
                return this->internalHandle.idx == other.internalHandle.idx;
            }

            inline static TextureSamplerHandle null()
            {
                TextureSamplerHandle ret;
                ret.internalHandle.idx = bgfx::kInvalidHandle;
                return ret;
            }

            friend class Firework::GL::Renderer;
        private:
            bgfx::UniformHandle internalHandle { bgfx::kInvalidHandle };
        };

        struct __firework_gl_api Texture2DHandle final
        {
            static Texture2DHandle create
            (
                const void* textureData, uint32_t textureDataSize,
                uint16_t width, uint16_t height,
                bool hasMipMaps = false, uint16_t layerCount = 1,
                bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8,
                uint64_t flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE
            );
            static Texture2DHandle create(const unsigned char (&color)[4]);
            void destroy();

            friend class Firework::GL::Renderer;
        private:
            bgfx::TextureHandle internalHandle;
        };
    }
}