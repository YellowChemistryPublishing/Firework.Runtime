#include "Light.h"

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;

TextureSamplerHandle DirectionalLightManager::directionalLightsSampler;
Texture2DHandle DirectionalLightManager::directionalLightsAcc;
uint32_t DirectionalLightManager::directionalLightsAccMaxLength = 0;
uint32_t DirectionalLightManager::directionalLightsAccLength = 0;
std::vector<DirectionalLightData> DirectionalLightManager::directionalLights;

void DirectionalLightManager::ctor()
{
    DirectionalLightManager::directionalLightsSampler = TextureSamplerHandle::create("s_directionalLights");

    // TODO: Maybe don't hardcode initial size of vec?
    DirectionalLightManager::directionalLights.push_back(DirectionalLightData { -0.01f, 1.0f, -0.01f, 0, 1, 1, 1, 0 });
    DirectionalLightManager::directionalLightsAcc = Texture2DHandle::createDynamic(2, 1, false, 1, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_BLIT_DST);
    DirectionalLightManager::directionalLightsAcc.updateDynamic(DirectionalLightManager::directionalLights.data(), sizeof(DirectionalLightData) * DirectionalLightManager::directionalLights.size(), 0, 0, 0, 0, 2, 1);
    DirectionalLightManager::directionalLightsAccMaxLength = 32;
}
void DirectionalLightManager::dtor()
{
    DirectionalLightManager::directionalLightsSampler.destroy();
}
uint16_t DirectionalLightManager::pushDirectionalLight(const Mathematics::Vector3& pos, const Mathematics::Vector3& color)
{
    if (DirectionalLightManager::directionalLightsAccLength >= DirectionalLightManager::directionalLightsAccMaxLength)
    {
        Texture2DHandle newTexture = Texture2DHandle::createDynamic(2, DirectionalLightManager::directionalLightsAccMaxLength *= 2, false, 1, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_BLIT_DST);
    }

    DirectionalLightManager::directionalLights.emplace_back(DirectionalLightData { pos.x, pos.y, pos.z, color.x, color.y, color.z });

    return 0; // FIXME.
}
void DirectionalLightManager::popDirectionalLight(uint16_t index)
{

}

DirectionalLightHandle DirectionalLightHandle::create(const Mathematics::Vector3& pos, const Mathematics::Vector3& color)
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
