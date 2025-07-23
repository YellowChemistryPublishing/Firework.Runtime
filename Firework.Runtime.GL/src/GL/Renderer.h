#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <glm/gtc/quaternion.hpp>
#include <module/sys>

namespace Firework::GL
{
    struct Uniform;
    struct TextureSampler;
    struct Texture2D;
    class GeometryProgram;

    class StaticMesh;
    class DynamicMesh;

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

    class _fw_gl_api Renderer final
    {
        static std::vector<std::pair<void (*)(bgfx::ViewId, void*), void*>> drawPassIntercepts;
    public:
        Renderer() = delete;

        static bool initialize(void* ndt, void* nwh, u32 width, u32 height, RendererBackend backend = RendererBackend::Default, u32 initFlags = BGFX_RESET_NONE);
        static void shutdown();

        static void showDebugInformation(bool visible = true);
        static void showDebugWireframes(bool visible = true);

        static RendererBackend rendererBackend();
        static std::vector<RendererBackend> platformBackends();

        static void setViewOrthographic(bgfx::ViewId id, float width, float height, glm::vec3 position = { 0.0f, 0.0f, 0.0f }, glm::quat rotation = { 1.0f, 0.0f, 0.0f, 0.0f },
                                        float near = -1.0f, float far = 2048.0f);
        static void setViewPerspective(bgfx::ViewId id, float width, float height, float yFieldOfView = 60.0f, glm::vec3 position = { 0.0f, 0.0f, 0.0f },
                                       glm::quat rotation = { 1.0f, 0.0f, 0.0f, 0.0f }, float near = 0.0f, float far = 2048.0f);

        static void setViewClear(bgfx::ViewId id, u32 rgbaColor = 0x000000ff, u16 flags = BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, float depth = 1.0f,
                                 byte stencil = 0);
        static void setViewArea(bgfx::ViewId id, u16 x, u16 y, u16 width, u16 height);
        static void setViewDrawOrder(bgfx::ViewId id, bgfx::ViewMode::Enum order);

        static void resetBackbuffer(u32 width, u32 height, u32 flags = BGFX_RESET_NONE, bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count);

        static void setDrawTransform(const glm::mat4& transform);
        [[nodiscard]] static bool setDrawUniform(const Uniform& uniform, const void* data);
        [[nodiscard]] static bool setDrawArrayUniform(const Uniform& uniform, const void* data, u16 count);
        [[nodiscard]] static bool setDrawTexture(u8 stage, const Texture2D& texture, const TextureSampler& sampler, u32 flags = std::numeric_limits<u32::underlying_type>::max());
        static void setDrawStencil(u32 func, u32 back = BGFX_STENCIL_NONE);

        static void addDrawPassIntercept(void (*intercept)(bgfx::ViewId, void*), void* data = nullptr);
        static void removeDrawPassIntercept(void (*intercept)(bgfx::ViewId, void*));

        [[nodiscard]] static bool submitDraw(bgfx::ViewId id, const StaticMesh& mesh, const GeometryProgram& program,
                                             u64 state = BGFX_STATE_NONE | BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA |
                                                 BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS,
                                             u32 blendFactor = 0);
        [[nodiscard]] static bool submitDraw(bgfx::ViewId id, const DynamicMesh& mesh, const GeometryProgram& program,
                                             u64 state = BGFX_STATE_NONE | BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA |
                                                 BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS,
                                             u32 blendFactor = 0);

#if _DEBUG
        static void debugDrawCube(glm::vec3 position, float sideLength = 1.0f);
#else
        inline static void debugDrawCube(glm::vec3, float)
        { }
#endif

        static void drawFrame();
    };
} // namespace Firework::GL
