#pragma once

#include "Firework.Runtime.RenderPipeline.Exports.h"

#include <GL/ModelLoader.h>
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
        static void (*resetViewArea)(uint16_t w, uint16_t h);
        /// @internal
        /// @brief Low-level API. Function called to resize the backbuffer. Defaults to ```Firework::GL::RenderPipeline::defaultResetBackbuffer```.
        /// @note Render thread only.
        static void (*resetBackbuffer)(uint32_t w, uint32_t h);
        /// @internal
        /// @brief Low-level API. Function called to render the current frame. Defaults to ```Firework::GL::RenderPipeline::defaultRenderFrame```.
        /// @note Render thread only.
        static void (*renderFrame)();

        static void defaultClearViewArea();
        static void defaultResetViewArea(uint16_t w, uint16_t h);
        static void defaultResetBackbuffer(uint32_t w, uint32_t h);
        static void defaultRenderFrame();
    };
} // namespace Firework::GL