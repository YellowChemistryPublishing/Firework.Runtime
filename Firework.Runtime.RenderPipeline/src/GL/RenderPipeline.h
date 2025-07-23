#pragma once

#include "Firework.Runtime.RenderPipeline.Exports.h"

#include <module/sys>

#include <GL/Shader.h>

namespace Firework::GL
{
    class _fw_rp_api RenderPipeline final
    {
    public:
        RenderPipeline() = delete;

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

        static void defaultClearViewArea();
        static void defaultResetViewArea(u16 w, u16 h);
        static void defaultResetBackbuffer(u32 w, u32 h);
        static void defaultRenderFrame();
    };
} // namespace Firework::GL