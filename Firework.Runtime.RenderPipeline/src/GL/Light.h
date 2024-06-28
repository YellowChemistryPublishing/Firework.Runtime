#pragma once

#include "Firework.Runtime.RenderPipeline.Exports.h"

#include <cstdint>
#include <vector>

#include <Mathematics.h>
#include <GL/Texture.h>
#include <GL/TextureVector.h>

namespace Firework
{
    namespace GL
    {
        class RenderPipeline;
        struct DirectionalLightHandle;

        class __firework_rp_api DirectionalLightManager final
        {
            static TextureSamplerHandle directionalLightsSampler;
            static TextureVector<2>* directionalLights;
            
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