#include "Texture.h"

#include <cstring>
#include <stdint.h>

using namespace Firework::GL;

TextureSampler::TextureSampler(std::string_view name)
{
    std::string cName(name.begin(), name.end());
    this->internalHandle = bgfx::createUniform(cName.c_str(), bgfx::UniformType::Sampler);
}
_fw_gl_common_handle_dtor(TextureSampler);

Texture2D::Texture2D(const std::span<const byte> textureData, const u16 width, const u16 height, bool hasMipMaps, const u16 layerCount, bgfx::TextureFormat::Enum format,
                     const u64 flags) :
    internalHandle(bgfx::createTexture2D(+width, +height, hasMipMaps, +layerCount, format, +flags, bgfx::copy(textureData.data(), textureData.size_bytes())))
{ }
Texture2D::Texture2D(const u16 width, const u16 height, bool hasMipMaps, const u16 layerCount, bgfx::TextureFormat::Enum format, const u64 flags) :
    internalHandle(bgfx::createTexture2D(+width, +height, hasMipMaps, +layerCount, format, +flags))
{ }

void Texture2D::updateDynamic(const std::span<const byte> textureData, const u16 layer, const u8 mip, const u16 x, const u16 y, const u16 width, const u16 height)
{
    bgfx::updateTexture2D(this->internalHandle, +layer, +mip, +x, +y, +width, +height, bgfx::copy(textureData.data(), textureData.size_bytes()));
}
void Texture2D::copyTo(const Texture2D& dest, const u16 dstX, const u16 dstY, const u16 srcX, const u16 srcY, const u16 width, const u16 height)
{
    this->copyTo(0, dest, dstX, dstY, srcX, srcY, width, height);
}
void Texture2D::copyTo(const u16 view, const Texture2D& dest, const u16 dstX, const u16 dstY, const u16 srcX, const u16 srcY, const u16 width, const u16 height)
{
    bgfx::blit(+view, dest.internalHandle, +dstX, +dstY, this->internalHandle, +srcX, +srcY, +width, +height);
}
