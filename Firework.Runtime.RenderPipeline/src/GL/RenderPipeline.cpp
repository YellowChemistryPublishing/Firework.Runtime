#include "RenderPipeline.h"

#include <GL/Framebuffer.h>
#include <GL/Geometry.h>
#include <GL/Renderer.h>
#include <GL/Shader.h>
#include <GL/Texture.h>

#include <Draw.vfAll.h>

using namespace Firework;
using namespace Firework::GL;

Texture2D RenderPipeline::framebufferAttachments[2] { nullptr, nullptr };
Framebuffer RenderPipeline::framebuffer = nullptr;

StaticMesh RenderPipeline::unitSquare = nullptr;
TextureSampler RenderPipeline::framebufferSampler = nullptr;
GeometryProgram RenderPipeline::drawFramebuffer = nullptr;

void (*RenderPipeline::clearViewArea)() = RenderPipeline::defaultClearViewArea;
void (*RenderPipeline::resetViewArea)(u16, u16) = RenderPipeline::defaultResetViewArea;
void (*RenderPipeline::resetBackbuffer)(u32, u32) = RenderPipeline::defaultResetBackbuffer;
void (*RenderPipeline::renderFrame)() = RenderPipeline::defaultRenderFrame;

bool RenderPipeline::renderInitialize(void* ndt, void* nwh, u32 w, u32 h, RendererBackend be)
{
    if (!Renderer::initialize(ndt, nwh, w, h, be))
        return false;

    RenderPipeline::framebufferAttachments[0] = Texture2D(BackbufferRatio::Equal, false, 1, TextureFormat::RGBA8, BGFX_TEXTURE_RT | BGFX_TEXTURE_RT_MSAA_X16);
    RenderPipeline::framebufferAttachments[1] =
        Texture2D(BackbufferRatio::Equal, false, 1, TextureFormat::D24S8, BGFX_TEXTURE_RT | BGFX_TEXTURE_RT_MSAA_X16 | BGFX_TEXTURE_RT_WRITE_ONLY);
    RenderPipeline::framebuffer = Framebuffer(RenderPipeline::framebufferAttachments);

    float unitSquareVerts[] {
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // [0]
        -1.0f, 1.0f,  1.0f, 0.0f, 1.0f, // [1]
        1.0f,  1.0f,  1.0f, 1.0f, 1.0f, // [2]
        1.0f,  -1.0f, 1.0f, 1.0f, 0.0f  // [3]
    };
    uint16_t unitSquareInds[] { 2, 1, 0, 3, 2, 0 };
    RenderPipeline::unitSquare =
        StaticMesh(std::span(_asr(byte*, &unitSquareVerts), sizeof(unitSquareVerts)),
                   VertexLayout(std::array { VertexDescriptor { .attribute = VertexAttributeName::Position, .type = VertexAttributeType::Float, .count = 3 },
                                             VertexDescriptor { .attribute = VertexAttributeName::TexCoord0, .type = VertexAttributeType::Float, .count = 2 } }),
                   std::span(unitSquareInds));

    RenderPipeline::framebufferSampler = TextureSampler("s_frame");
    createShaderFromPrecompiled(RenderPipeline::drawFramebuffer, Draw);

    RenderPipeline::resetViewArea(u16(w), u16(h));
    RenderPipeline::clearViewArea();

    return true;
}
void RenderPipeline::renderShutdown()
{
    RenderPipeline::drawFramebuffer = nullptr;
    RenderPipeline::framebufferSampler = nullptr;
    RenderPipeline::unitSquare = nullptr;

    RenderPipeline::framebuffer = nullptr;
    RenderPipeline::framebufferAttachments[1] = nullptr;
    RenderPipeline::framebufferAttachments[0] = nullptr;

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
    (void)Renderer::setViewFramebuffer(1, RenderPipeline::framebuffer);

    Renderer::setViewFramebuffer(0, nullptr);
    (void)Renderer::setDrawTexture(0, RenderPipeline::framebuffer, RenderPipeline::framebufferSampler);
    (void)Renderer::submitDraw(0, RenderPipeline::unitSquare, RenderPipeline::drawFramebuffer,
                               BGFX_STATE_NONE | BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA | BGFX_STATE_DEPTH_TEST_ALWAYS);

    Renderer::drawFrame();
}
