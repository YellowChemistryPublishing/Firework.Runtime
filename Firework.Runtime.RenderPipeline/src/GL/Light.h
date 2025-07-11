#pragma once

#include "Firework.Runtime.RenderPipeline.Exports.h"

#include <cstdint>
#include <module/sys.Mathematics>
#include <vector>

#include <GL/Texture.h>
#include <GL/TextureVector.h>

namespace Firework
{
    namespace GL
    {
        class RenderPipeline;
        struct DirectionalLightHandle;

        class _fw_rp_api DirectionalLightManager final
        {
            static TextureSamplerHandle directionalLightsSampler;
            static TextureVector<2>* directionalLights;

            static void ctor();
            static void dtor();
            static uint16_t pushDirectionalLight(const sysm::vector3& pos, const sysm::vector3& color);
            static void popDirectionalLight(uint16_t index);
        public:
            friend class Firework::GL::RenderPipeline;
            friend struct Firework::GL::DirectionalLightHandle;
        };

        struct DirectionalLightHandle
        {
            static DirectionalLightHandle create(const sysm::vector3& pos, const sysm::vector3& color);
            void update(const sysm::vector3& pos, const sysm::vector3& color);
            void destroy();
        private:
            uint16_t index = UINT16_MAX;
        };
    } // namespace GL
} // namespace Firework