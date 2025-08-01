#pragma once

#include "Firework.Runtime.GL.Exports.h"

_push_nowarn_clang(_clWarn_clang_zero_as_nullptr);
#include <bgfx/bgfx.h>
#include <glm/gtc/quaternion.hpp>
#include <module/sys>
_pop_nowarn_clang();

#include <GL/TextureFormat.h>

_push_nowarn_msvc(_clWarn_msvc_export_interface);
namespace Firework::GL
{
    struct Uniform;
    struct TextureSampler;
    struct Texture2D;
    struct Framebuffer;

    class StaticMesh;
    class DynamicMesh;
    class GeometryProgram;

    using ViewIndex = bgfx::ViewId;

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

    enum class DrawOrder
    {
        Default = bgfx::ViewMode::Default,
        Sequential = bgfx::ViewMode::Sequential,
        DepthAscending = bgfx::ViewMode::DepthAscending,
        DepthDescending = bgfx::ViewMode::DepthDescending
    };

    class _fw_gl_api Renderer final
    {
        static std::vector<std::pair<void (*)(ViewIndex, void*), void*>> drawPassIntercepts;
    public:
        Renderer() = delete;

        static bool initialize(void* ndt, void* nwh, u32 width, u32 height, RendererBackend backend = RendererBackend::Default, u32 initFlags = BGFX_RESET_NONE);
        static void shutdown();

        static void showDebugInformation(bool visible = true);
        static void showDebugWireframes(bool visible = true);

        static RendererBackend rendererBackend();
        static std::vector<RendererBackend> platformBackends();

        static void setViewOrthographic(ViewIndex id, float width, float height, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                                        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), float near = -1.0f, float far = 2048.0f);
        static void setViewPerspective(ViewIndex id, float width, float height, float yFieldOfView = 60.0f, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                                       glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), float near = 0.0f, float far = 2048.0f);

        static void setViewClear(ViewIndex id, u32 rgbaColor = 0x000000ff, u16 flags = BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, float depth = 1.0f,
                                 byte stencil = 0);
        static void setViewArea(ViewIndex id, u16 x, u16 y, u16 width, u16 height);
        static void setViewDrawOrder(ViewIndex id, bgfx::ViewMode::Enum order);
        static void setViewFramebuffer(ViewIndex id, const Framebuffer& framebuffer);

        static void resetBackbuffer(u32 width, u32 height, u32 flags = BGFX_RESET_NONE, TextureFormat format = TextureFormat::Count);

        static void setDrawTransform(const glm::mat4& transform);
        [[nodiscard]] static bool setDrawUniform(const Uniform& uniform, const void* data);
        [[nodiscard]] static bool setDrawArrayUniform(const Uniform& uniform, const void* data, u16 count);
        [[nodiscard]] static bool setDrawTexture(u8 stage, const Texture2D& texture, const TextureSampler& sampler, u32 flags = std::numeric_limits<u32::underlying_type>::max());
        [[nodiscard]] static bool setDrawTexture(u8 stage, const Framebuffer& framebuffer, const TextureSampler& sampler, u8 attachmentIndex = 0,
                                                 u32 flags = std::numeric_limits<u32::underlying_type>::max());
        static void setDrawStencil(u32 func, u32 back = BGFX_STENCIL_NONE);

        static void addDrawPassIntercept(void (*intercept)(ViewIndex, void*), void* data = nullptr);
        static void removeDrawPassIntercept(void (*intercept)(ViewIndex, void*));

        _push_nowarn_gcc(_clWarn_gcc_c_cast);
        _push_nowarn_clang(_clWarn_clang_c_cast);
        [[nodiscard]] static bool submitDraw(ViewIndex id, const StaticMesh& mesh, const GeometryProgram& program,
                                             u64 state = BGFX_STATE_NONE | BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA |
                                                 BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS,
                                             u32 blendFactor = 0);
        [[nodiscard]] static bool submitDraw(ViewIndex id, const DynamicMesh& mesh, const GeometryProgram& program,
                                             u64 state = BGFX_STATE_NONE | BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA |
                                                 BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS,
                                             u32 blendFactor = 0);
        _pop_nowarn_clang();
        _pop_nowarn_gcc();

#if _DEBUG
        static void debugDrawCube(glm::vec3 position, float sideLength = 1.0f);
#else
        inline static void debugDrawCube(glm::vec3, float)
        { }
#endif

        static void drawFrame();
    };
} // namespace Firework::GL
_pop_nowarn_msvc();
