#include "FringeRenderer.h"

#include <EntityComponentSystem/EngineEvent.h>
#include <GL/Renderer.h>
#include <GL/Shader.h>
#include <Library/Math.h>

#include <Colored.vfAll.h>

using namespace Firework;
using namespace Firework::GL;
using namespace Firework::Internal;

GeometryProgram FringeRenderer::drawProgram = nullptr;

bool FringeRenderer::renderInitialize()
{
    InternalEngineEvent::OnRenderShutdown += []
    {
        if (FringeRenderer::drawProgram) [[likely]]
            FringeRenderer::drawProgram = nullptr;
    };

    createShaderFromPrecompiled(FringeRenderer::drawProgram, Colored, std::array { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });

    return FringeRenderer::drawProgram;
}

FringeRenderer::FringeRenderer(const std::span<const FringePoint> points, const std::span<const ssz> closedPathRanges)
{
    _fence_value_return(, points.size() < 2);
    _fence_value_return(, closedPathRanges.size() < 2);

    std::vector<FringePoint> verts;
    verts.reserve(+(sz(points.size()) + 1_uz));
    verts.insert(verts.begin(), points.begin(), points.end());
    if (points.front() != points.back())
        verts.emplace_back(points.front());

    std::vector<uint16_t> inds;
    inds.reserve(+(sz(verts.size()) * 2_uz));

    for (auto boundBegIt = closedPathRanges.cbegin(); boundBegIt != --closedPathRanges.cend(); ++boundBegIt)
    {
        auto boundEndIt = ++decltype(boundBegIt)(boundBegIt);
        for (u16 i = u16(*boundBegIt); i < u16(*boundEndIt) - 1_u16; i++)
        {
            inds.emplace_back(+i);
            inds.emplace_back(+(i + 1_u16));
        }
    }

    this->fill = StaticMesh(std::span(_asr(byte*, verts.data()), verts.size() * sizeof(decltype(verts)::value_type)),
                            VertexLayout(std::array { VertexDescriptor { .attribute = VertexAttributeName::Position, .type = VertexAttributeType::Float, .count = 3 } }), inds);
}

bool FringeRenderer::submitDraw(const float renderIndex, const glm::mat4 transform, const Color color) const
{
    _fence_value_return(false, !this->fill || !FringeRenderer::drawProgram);

    float colUniform[4] { float(color.r) / 255.0f, float(color.g) / 255.0f, float(color.b) / 255.0f, float(color.a) / 255.0f };
    (void)FringeRenderer::drawProgram.setUniform("u_color", &colUniform);

    glm::mat4 shapeTransform = glm::translate(glm::mat4(1.0f), LinAlgConstants::forward * renderIndex);
    shapeTransform *= transform;
    Renderer::setDrawTransform(shapeTransform);

    (void)Renderer::submitDraw(1, this->fill, FringeRenderer::drawProgram,
                               BGFX_STATE_NONE | BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA | BGFX_STATE_WRITE_Z |
                                   BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_PT_LINES | BGFX_STATE_LINEAA);

    return true;
}
