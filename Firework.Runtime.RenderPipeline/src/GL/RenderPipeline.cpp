#include "RenderPipeline.h"

#include <chrono>
#include <module/sys.Mathematics>
#include <thread>

#include <GL/Renderer.h>

using namespace Firework;
using namespace Firework::GL;

static GeometryProgramHandle genericProgram;

void (*RenderPipeline::clearViewArea)() = RenderPipeline::defaultClearViewArea;
void (*RenderPipeline::resetViewArea)(uint16_t, uint16_t) = RenderPipeline::defaultResetViewArea;
void (*RenderPipeline::resetBackbuffer)(uint32_t, uint32_t) = RenderPipeline::defaultResetBackbuffer;
void (*RenderPipeline::renderFrame)() = RenderPipeline::defaultRenderFrame;

void RenderPipeline::defaultClearViewArea()
{
    Renderer::setViewClear(0, 0x00000000, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL);
}
void RenderPipeline::defaultResetViewArea(uint16_t w, uint16_t h)
{
    Renderer::setViewArea(0, 0, 0, w, h);
    Renderer::setViewArea(1, 0, 0, w, h);
    Renderer::setViewOrthographic(1, w, h, sysm::vector3(0, 0, 0), Renderer::fromEuler(sysm::vector3(0, 0, 0)), 0.0f, 16777216.0f);
    Renderer::setViewDrawOrder(1, bgfx::ViewMode::Sequential);
}
void RenderPipeline::defaultResetBackbuffer(uint32_t w, uint32_t h)
{
    Renderer::resetBackbuffer(w, h);
}
void RenderPipeline::defaultRenderFrame()
{
    Renderer::drawFrame();
}
