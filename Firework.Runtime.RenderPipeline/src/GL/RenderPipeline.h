#pragma once

#include "Firework.Runtime.RenderPipeline.Exports.h"

#include <GL/ModelLoader.h>
#include <GL/Shader.h>

namespace Firework
{
    namespace GL
    {
        template <typename MeshHandleType>
        struct DrawTask
        {
            bgfx::ViewId id;
            MeshHandleType mesh;
            GeometryProgramHandle program;
            uint64_t state = BGFX_STATE_NONE | BGFX_STATE_CULL_CW     | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                                               BGFX_STATE_BLEND_ALPHA | BGFX_STATE_WRITE_Z   | BGFX_STATE_DEPTH_TEST_LESS;
            uint32_t blendFactor = 0;
        };

        class __firework_rp_api RenderPipeline final
        {
        public:
            RenderPipeline() = delete;

            static void init();
            static void deinit();

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

            static void drawMesh(GL::StaticMeshHandle mesh);
        };
    }
}