#include "Texture.h"

#include <cstring>

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
        bgfx::makeRef(texData, textureDataSize, [](void* data, void*) { delete[] (char*)data; })
    );
    return ret;
}
Texture2DHandle Texture2DHandle::create(const unsigned char (&color)[4])
{
    Texture2DHandle ret;
    ret.internalHandle = bgfx::createTexture2D
    (
        1, 1, false, 1, bgfx::TextureFormat::RGBA8,
        BGFX_TEXTURE_NONE | BGFX_SAMPLER_COMPARE_LESS | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT, bgfx::copy(color, sizeof(color))
    );
    return ret;
}
void Texture2DHandle::destroy()
{
    bgfx::destroy(this->internalHandle);
}