#include "Texture.h"

#include <cstring>
#include <stdint.h>

using namespace Firework::GL;

TextureSamplerHandle TextureSamplerHandle::create(const char* name)
{
    TextureSamplerHandle ret;
    ret.internalHandle = bgfx::createUniform(name, bgfx::UniformType::Sampler);
    return ret;
}
void TextureSamplerHandle::destroy()
{
    bgfx::destroy(this->internalHandle);
}

Texture2DHandle Texture2DHandle::create
(
    const void* textureData, uint32_t textureDataSize,
    uint16_t width, uint16_t height,
    bool hasMipMaps, uint16_t layerCount,
    bgfx::TextureFormat::Enum format, uint64_t flags
)
{
    Texture2DHandle ret;
    char* texData = new char[textureDataSize];
    memcpy(texData, textureData, textureDataSize);
    ret.internalHandle = bgfx::createTexture2D
    (
        width, height, hasMipMaps, layerCount, format, flags,
        bgfx::makeRef(texData, textureDataSize, [](void* data, void*) { delete[] static_cast<char*>(data); })
    );
    return ret;
}
Texture2DHandle Texture2DHandle::create(const unsigned char (&color)[4])
{
    Texture2DHandle ret;
    ret.internalHandle = bgfx::createTexture2D
    (
        1, 1, false, 1, bgfx::TextureFormat::RGBA8,
        BGFX_TEXTURE_NONE | BGFX_SAMPLER_COMPARE_LESS | BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC, bgfx::copy(color, sizeof(color))
    );
    return ret;
}
Texture2DHandle Texture2DHandle::createDynamic
(
    uint16_t width, uint16_t height,
    bool hasMipMaps, uint16_t layerCount,
    bgfx::TextureFormat::Enum format, uint64_t flags
)
{
    Texture2DHandle ret;
    ret.internalHandle = bgfx::createTexture2D(width, height, hasMipMaps, layerCount, format, flags);
    return ret;
}
void Texture2DHandle::updateDynamic(const void* textureData, uint32_t textureDataSize, uint16_t layer, uint8_t mip, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    bgfx::updateTexture2D
    (
        this->internalHandle, layer, mip, x, y, width, height,
        bgfx::copy(textureData, textureDataSize)
    );
}
void Texture2DHandle::copyTo(Texture2DHandle dest, uint16_t dstX, uint16_t dstY, uint16_t srcX, uint16_t srcY, uint16_t width, uint16_t height)
{
    bgfx::blit(0, dest.internalHandle, dstX, dstY, this->internalHandle, srcX, srcY, width, height);
}
void Texture2DHandle::copyTo(uint16_t view, Texture2DHandle dest, uint16_t dstX, uint16_t dstY, uint16_t srcX, uint16_t srcY, uint16_t width, uint16_t height)
{
    bgfx::blit(view, dest.internalHandle, dstX, dstY, this->internalHandle, srcX, srcY, width, height);
}
void Texture2DHandle::destroy()
{
    bgfx::destroy(this->internalHandle);
}