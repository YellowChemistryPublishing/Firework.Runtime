#include "RenderPipeline.h"

#include <chrono>
#include <thread>

#include <GL/Renderer.h>

using namespace Firework;
using namespace Firework::GL;

static GeometryProgramHandle genericProgram;

void (*RenderPipeline::clearViewArea)() = RenderPipeline::defaultClearViewArea;
void (*RenderPipeline::resetViewArea)(u16, u16) = RenderPipeline::defaultResetViewArea;
void (*RenderPipeline::resetBackbuffer)(u32, u32) = RenderPipeline::defaultResetBackbuffer;
void (*RenderPipeline::renderFrame)() = RenderPipeline::defaultRenderFrame;

void RenderPipeline::defaultClearViewArea()
{
    Renderer::setViewClear(0, 0x00000000, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL);
}
void RenderPipeline::defaultResetViewArea(u16 w, u16 h)
{
    Renderer::setViewArea(0, 0, 0, w, h);
    Renderer::setViewArea(1, 0, 0, w, h);
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
