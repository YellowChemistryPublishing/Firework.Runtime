#pragma once

#include "GL.Config.h"
#include "Firework.Runtime.GL.Exports.h"

#include <cmath>
#include <cstdint>
#include <list>
#include <vector>

#include <GL/Geometry.h>
#include <GL/Shader.h>

namespace Firework
{
    namespace Internal
    {
        struct DirectionalLight
        {
            union
            {
                struct
                {
                    // Nuclear [[maybe_unused]] attr.
                    [[maybe_unused]] float
                    xDir, yDir, zDir, _discard1,
                    rAmbient, gAmbient, bAmbient, _discard2,
                    rDiffuse, gDiffuse, bDiffuse, _discard3,
                    rSpecular, gSpecular, bSpecular, _discard4;
                };
                struct
                {
                    float dir[3];
                    [[maybe_unused]] float _discard5;
                    float ambient[3];
                    [[maybe_unused]] float _discard6;
                    float diffuse[3];
                    [[maybe_unused]] float _discard7;
                    float specular[3];
                };
                float data[16];
            };
        };
        struct PointLight
        {
            union
            {
                struct
                {
                    float x, y, z, _discard1,
                    rAmbient, gAmbient, bAmbient, constant,
                    rDiffuse, gDiffuse, bDiffuse, linear,
                    rSpecular, gSpecular, bSpecular, quadratic;
                };
                struct
                {
                    float pos[3];
                    float _discard2;
                    float ambient[3];
                    float _discard3;
                    float diffuse[3];
                    float _discard4;
                    float specular[3];
                };
                float data[16];
            };
        };

        static_assert(sizeof(DirectionalLight) == sizeof(float[16]), "This shouldn't happen!");
    }

    namespace GL
    {
        class Renderer;
        class DirectionalLightHandle;
        class StaticLitObjectHandle;

        enum class LightingModel : uint_fast8_t
        {
            Generic, // Phong lighting.
            PhysicallyBased // PBR.
        };

        class __firework_gl_api Lighting final
        {
            static GeometryProgramHandle lightingProgram;
            static std::list<Internal::DirectionalLight> dirLights;
            static std::list<Internal::PointLight> pointLights;

            inline static void init()
            {
                Lighting::lightingProgram = Lighting::createProgramGeneric();
            }
            inline static void deinit()
            {
                Lighting::lightingProgram.destroy();
            }
            static GeometryProgramHandle createProgramGeneric();
        public:
            Lighting() = delete;

            static void setLightingModel(LightingModel lm);

            friend class Firework::GL::Renderer;
            friend class Firework::GL::DirectionalLightHandle;
            friend class Firework::GL::StaticLitObjectHandle;
        };

        class __firework_gl_api DirectionalLightHandle final
        {
            std::list<Internal::DirectionalLight>::iterator it;
        public:
            inline static DirectionalLightHandle create(const float (&dir)[3], const float (&ambient)[3], const float (&diffuse)[3], const float (&specular)[3])
            {
                DirectionalLightHandle ret;

                Lighting::dirLights.push_back(Internal::DirectionalLight
                {
                    .xDir = dir[0], .yDir = dir[1], .zDir = dir[2],
                    .rAmbient = ambient[0], .gAmbient = ambient[1], .bAmbient = ambient[2],
                    .rDiffuse = diffuse[0], .gDiffuse = diffuse[1], .bDiffuse = diffuse[2],
                    .rSpecular = specular[0], .gSpecular = specular[1], .bSpecular = specular[2],
                });
                ret.it = --Lighting::dirLights.end();

                float len = sqrtf(ret.it->xDir * ret.it->xDir + ret.it->yDir * ret.it->yDir + ret.it->zDir * ret.it->zDir);
                ret.it->xDir = -ret.it->xDir / len; ret.it->yDir = -ret.it->yDir / len; ret.it->zDir = -ret.it->zDir / len;

                return ret;
            }
            inline void destroy()
            {
                this->it->xDir = 0.0f;
                this->it->yDir = -1.0f;
                this->it->zDir = 0.0f;
                this->it->rAmbient = 0.0f;
                this->it->gAmbient = 0.0f;
                this->it->bAmbient = 0.0f;
                this->it->rDiffuse = 0.0f;
                this->it->gDiffuse = 0.0f;
                this->it->bDiffuse = 0.0f;
                this->it->rSpecular = 0.0f;
                this->it->gSpecular = 0.0f;
                this->it->bSpecular = 0.0f;
            }
        };
    }
}