#pragma once

#include "Firework.Runtime.RenderPipeline.Exports.h"

#include <cstdint>
#include <vector>

#include <Mathematics.h>
#include <GL/Texture.h>

namespace Firework
{
    namespace Internal
    {
        #pragma pack(push, 1)
        struct DirectionalLightData
        {
            float data[8];
            // The texture data should have this spec.
            // [ x, ... ] |
            // [ y, ... ] |
            // [ z, ... ] |
            // [ _, ... ] |
            // [ r, ... ] |
            // [ g, ... ] |
            // [ b, ... ] |
            // [ _, ... ] v (h = 2 * 4)
        };
        #pragma pack(pop)
    }

    namespace GL
    {
        class RenderPipeline;
        struct DirectionalLightHandle;

        class __firework_rp_api DirectionalLightManager final
        {
            static TextureSamplerHandle directionalLightsSampler;
            static Texture2DHandle directionalLightsAcc;
            static uint32_t directionalLightsAccMaxLength;
            static uint32_t directionalLightsAccLength;
            static std::vector<Internal::DirectionalLightData> directionalLights;

            static void ctor();
            static void dtor();
            static uint16_t pushDirectionalLight(const Mathematics::Vector3& pos, const Mathematics::Vector3& color);
            static void popDirectionalLight(uint16_t index);
        public:
            friend class Firework::GL::RenderPipeline;
            friend struct Firework::GL::DirectionalLightHandle;
        };
        
        struct DirectionalLightHandle
        {
            static DirectionalLightHandle create(const Mathematics::Vector3& pos, const Mathematics::Vector3& color);
            void update(const Mathematics::Vector3& pos, const Mathematics::Vector3& color);
            void destroy();
        private:
            uint16_t index = UINT16_MAX;
        };
    }
}