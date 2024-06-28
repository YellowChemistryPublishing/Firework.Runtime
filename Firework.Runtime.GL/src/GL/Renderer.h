#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>

#include <Mathematics.h>
#include <GL/Geometry.h>
#include <GL/Shader.h>
#include <GL/Texture.h>
#include <GL/TextureVector.h>
#include <GL/Transform.h>

namespace Firework
{
    namespace GL
    {
        enum class RendererBackend
        {
			NoOp = bgfx::RendererType::Noop,
			AGC = bgfx::RendererType::Agc,
			Direct3D11 = bgfx::RendererType::Direct3D11,
			Direct3D12 = bgfx::RendererType::Direct3D12,
			GNM = bgfx::RendererType::Gnm,
			Metal = bgfx::RendererType::Metal,
			NVN = bgfx::RendererType::Nvn,
			OpenGLES = bgfx::RendererType::OpenGLES,
			OpenGL = bgfx::RendererType::OpenGL,
			Vulkan = bgfx::RendererType::Vulkan,
            Default = bgfx::RendererType::Count
        };

        class __firework_gl_api Renderer final
        {
        public:
            Renderer() = delete;

            static bool initialize(void* ndt, void* nwh, uint32_t width, uint32_t height, RendererBackend backend = RendererBackend::Default, uint32_t initFlags = BGFX_RESET_NONE);
            static void shutdown();

            static void showDebugInformation();
            static void hideDebugInformation();

            static RendererBackend rendererBackend();
            static std::vector<RendererBackend> platformBackends();

            static void setViewOrthographic
            (
                bgfx::ViewId id, float width, float height,
                Mathematics::Vector3 position = { 0.0f, 0.0f, 0.0f },
                Mathematics::Quaternion rotation = { 1.0f, 0.0f, 0.0f, 0.0f },
                float near = -1.0f, float far = 2048.0f
            );
            static void setViewPerspective
            (
                bgfx::ViewId id, float width, float height, float yFieldOfView = 60.0f,
                Mathematics::Vector3 position = { 0.0f, 0.0f, 0.0f },
                Mathematics::Quaternion rotation = { 1.0f, 0.0f, 0.0f, 0.0f },
                float near = 0.0f, float far = 2048.0f
            );
            static void setViewClear(bgfx::ViewId id, uint32_t rgbaColor = 0x000000ff, uint16_t flags = BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, float depth = 1.0f, uint8_t stencil = 0);
            static void setViewArea(bgfx::ViewId id, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
            static void resetBackbuffer(uint32_t width, uint32_t height, uint32_t flags = BGFX_RESET_NONE, bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count);

            static void setDrawTransform(const RenderTransform& transform);
            static void setDrawUniform(UniformHandle uniform, const void* data);
            static void setDrawArrayUniform(UniformHandle uniform, const void* data, uint16_t count);
            static void setDrawTexture(uint8_t stage, Texture2DHandle texture, TextureSamplerHandle sampler, uint64_t flags = BGFX_TEXTURE_NONE);
            template <uint16_t Vec4Count>
            inline static void setDrawTexture(uint8_t stage, const TextureVector<Vec4Count>& texture, TextureSamplerHandle sampler)
            {
                Renderer::setDrawTexture(stage, texture.gpuData, sampler, BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
            }
            static void submitDraw
            (
                bgfx::ViewId id, StaticMeshHandle mesh, GeometryProgramHandle program,
                uint64_t state = BGFX_STATE_NONE | BGFX_STATE_CULL_CW     | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                                                   BGFX_STATE_BLEND_ALPHA | BGFX_STATE_WRITE_Z   | BGFX_STATE_DEPTH_TEST_LESS,
                uint32_t blendFactor = 0
            );
            static void submitDraw
            (
                bgfx::ViewId id, DynamicMeshHandle mesh, GeometryProgramHandle program,
                uint64_t state = BGFX_STATE_NONE | BGFX_STATE_CULL_CW     | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                                                   BGFX_STATE_BLEND_ALPHA | BGFX_STATE_WRITE_Z   | BGFX_STATE_DEPTH_TEST_LESS,
                uint32_t blendFactor = 0
            );

            #if _DEBUG
            static void debugDrawCube(Mathematics::Vector3 position, float sideLength = 1.0f);
            #else
            inline static void debugDrawCube(Mathematics::Vector3, float)
            { }
            #endif

            static void drawFrame();

            static Mathematics::Quaternion fromEuler(Mathematics::Vector3 vec);
        };
    }
}