#pragma once

#include "LitObject.h"
#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <GL/Geometry.h>
#include <GL/Renderer.h>
#include <GL/Shader.h>
#include <GL/Texture.h>
#include <GL/Transform.h>

namespace Firework
{
    namespace GL
    {
        class Renderer;
        class StaticLitObjectHandle;

        struct LitVertex
        {
            float x, y, z;
            float tc0x, tc0y;

            inline static VertexLayout layout = VertexLayout::create({ { bgfx::Attrib::Position, bgfx::AttribType::Float, 3 }, { bgfx::Attrib::TexCoord0, bgfx::AttribType::Float, 2 } });
        };

        struct Material
        {
            float (&ambientStrength)[3] = reinterpret_cast<float(&)[3]>(this->data[0]);
            float (&diffuseStrength)[3] = reinterpret_cast<float(&)[3]>(this->data[4]);
            float (&specularStrength)[3] = reinterpret_cast<float(&)[3]>(this->data[8]);
            float& reflectivity = this->data[12];

            friend class Firework::GL::StaticLitObjectHandle;
        private:
            float data[16] { 1.0f };
        };

        class __firework_gl_api StaticLitObjectHandle final
        {
            StaticMeshHandle mesh;
            Texture2DHandle color;
            RenderTransform* internalTransform = new RenderTransform;
            Material* internalMaterial = new Material;

            static TextureSamplerHandle texColorSampler;

            inline static void init()
            {
                StaticLitObjectHandle::texColorSampler = TextureSamplerHandle::create("s_texColor");
            }
            inline static void deinit()
            {
                StaticLitObjectHandle::texColorSampler.destroy();
            }
        public:
            RenderTransform* transform;
            Material* material;

            inline static StaticLitObjectHandle create(StaticMeshHandle mesh, unsigned char color[4])
            {
                StaticLitObjectHandle ret;
                ret.mesh = mesh;
                ret.color = Texture2DHandle::create({ color[0], color[1], color[2], color[3] });
                ret.transform = ret.internalTransform;
                ret.material = ret.internalMaterial;
                return ret;
            }
            inline void destroy()
            {
                delete this->internalTransform;
                delete this->internalMaterial;
            }
            
            void submitDraw();

            friend class Firework::GL::Renderer;
        };
    }
}