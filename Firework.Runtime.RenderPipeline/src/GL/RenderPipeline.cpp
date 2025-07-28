#include "RenderPipeline.h"

#include <array>

#include <GL/Framebuffer.h>
#include <GL/Geometry.h>
#include <GL/Renderer.h>
#include <GL/Shader.h>
#include <GL/Texture.h>

#include <Draw.vfAll.h>

using namespace Firework;
using namespace Firework::GL;

void (*RenderPipeline::clearViewArea)() = RenderPipeline::defaultClearViewArea;
void (*RenderPipeline::resetViewArea)(u16, u16) = RenderPipeline::defaultResetViewArea;
void (*RenderPipeline::resetBackbuffer)(u32, u32) = RenderPipeline::defaultResetBackbuffer;
void (*RenderPipeline::renderFrame)() = RenderPipeline::defaultRenderFrame;

bool RenderPipeline::renderInitialize(void* ndt, void* nwh, u32 w, u32 h, RendererBackend be)
{
    if (!Renderer::initialize(ndt, nwh, w, h, be))
        return false;

    RenderPipeline::resetViewArea(u16(w), u16(h));
    RenderPipeline::clearViewArea();

    return true;
}
void RenderPipeline::renderShutdown()
{
    Renderer::shutdown();
}

void RenderPipeline::defaultClearViewArea()
{
    for (ViewIndex i = 0; i <= 1; i++) Renderer::setViewClear(i, 0x00000000, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL);
}
void RenderPipeline::defaultResetViewArea(u16 w, u16 h)
{
    for (ViewIndex i = 0; i <= 1; i++) Renderer::setViewArea(i, 0, 0, w, h);
    Renderer::setViewOrthographic(1, +w, +h, glm::vec3(0.0f), glm::quat(1.0f, glm::vec3(0.0f)), 0.0f, 16777216.0f);
    Renderer::setViewDrawOrder(1, bgfx::ViewMode::Sequential);
}
void RenderPipeline::defaultResetBackbuffer(u32 w, u32 h)
{
    Renderer::resetBackbuffer(w, h);
}
void RenderPipeline::defaultRenderFrame()
{
    Renderer::drawFrame();
}
