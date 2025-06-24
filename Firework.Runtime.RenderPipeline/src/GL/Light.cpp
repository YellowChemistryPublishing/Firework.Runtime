#include "Light.h"

using namespace Firework;
using namespace Firework::GL;

TextureSamplerHandle DirectionalLightManager::directionalLightsSampler;
TextureVector<2>* DirectionalLightManager::directionalLights;

void DirectionalLightManager::ctor()
{
    DirectionalLightManager::directionalLightsSampler = TextureSamplerHandle::create("s_directionalLights");

    DirectionalLightManager::directionalLights = new TextureVector<2>();
    // The texture data should have this spec.
    // [ x, ... ] |
    // [ y, ... ] |
    // [ z, ... ] |
    // [ _, ... ] |
    // [ r, ... ] |
    // [ g, ... ] |
    // [ b, ... ] |
    // [ _, ... ] v (h = 2 * 4)
    DirectionalLightManager::directionalLights->pushBack(TextureVectorElement<2> { .elementData = { { -0.01f, 1.0f, -0.01f, 0 }, { 1, 1, 1, 0 } } });
}
void DirectionalLightManager::dtor()
{
    delete DirectionalLightManager::directionalLights;
    DirectionalLightManager::directionalLightsSampler.destroy();
}
uint16_t DirectionalLightManager::pushDirectionalLight(const sysm::vector3& pos, const sysm::vector3& color)
{
    return 0;
}
void DirectionalLightManager::popDirectionalLight(uint16_t index)
{
}

DirectionalLightHandle DirectionalLightHandle::create(const sysm::vector3& pos, const sysm::vector3& color)
{
    DirectionalLightHandle ret;
    ret.index = DirectionalLightManager::pushDirectionalLight(pos, color);
    return ret;
}
void DirectionalLightHandle::destroy()
{
    DirectionalLightManager::popDirectionalLight(this->index);
    this->index = UINT16_MAX;
}
