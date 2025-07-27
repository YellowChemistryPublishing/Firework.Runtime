#pragma once

#include "Firework.Runtime.RenderPipeline.Exports.h"

#include <module/sys>

namespace Firework::GL
{
    enum class RendererBackend;

    class _fw_rp_api RenderPipeline final
    {
    public:
        static struct Texture2D framebufferAttachments[2]; // `TextureFormat::RGBA8`, `TextureFormat::D24S8`
        static struct Framebuffer framebuffer;

        static struct StaticMesh unitSquare;
        static struct TextureSampler framebufferSampler;
        static struct GeometryProgram drawFramebuffer;

        /// @internal
        /// @brief Low-level API. Function called to clear the view area. Defaults to ```Firework::GL::RenderPipeline::defaultClearViewArea```.
        /// @note Render thread only.
        static void (*clearViewArea)();
        /// @internal
        /// @brief Low-level API. Function called to set the view area. Defaults to ```Firework::GL::RenderPipeline::defaultResetViewArea```.
        /// @note Render thread only.
        static void (*resetViewArea)(u16 w, u16 h);
        /// @internal
        /// @brief Low-level API. Function called to resize the backbuffer. Defaults to ```Firework::GL::RenderPipeline::defaultResetBackbuffer```.
        /// @note Render thread only.
        static void (*resetBackbuffer)(u32 w, u32 h);
        /// @internal
        /// @brief Low-level API. Function called to render the current frame. Defaults to ```Firework::GL::RenderPipeline::defaultRenderFrame```.
        /// @note Render thread only.
        static void (*renderFrame)();

        RenderPipeline() = delete;

        [[nodiscard]] static bool renderInitialize(void* nwh, void* ndt, u32 w, u32 h, RendererBackend be);
        static void renderShutdown();

        static void defaultClearViewArea();
        static void defaultResetViewArea(u16 w, u16 h);
        static void defaultResetBackbuffer(u32 w, u32 h);
        static void defaultRenderFrame();
    };
} // namespace Firework::GL
