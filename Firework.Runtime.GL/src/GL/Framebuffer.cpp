#include "Framebuffer.h"

#include <GL/Texture.h>

using namespace Firework::GL;

Framebuffer::Framebuffer(const BackbufferRatio ratio, const TextureFormat format, const u64 flags) :
    internalHandle(bgfx::createFrameBuffer(_as(bgfx::BackbufferRatio::Enum, ratio), format, +flags))
{ }
Framebuffer::Framebuffer(const std::span<const Texture2D> textures)
{
    for (const Texture2D& texture : textures)
        if (!texture)
            return;

    std::vector<bgfx::TextureHandle> handles;
    handles.reserve(textures.size());
    std::transform(textures.begin(), textures.end(), std::back_inserter(handles), [](const Texture2D& texture) { return texture.internalHandle; });

    this->internalHandle = bgfx::createFrameBuffer(+u8(textures.size()), handles.data(), false);
}

_fw_gl_common_handle_dtor(Framebuffer);
