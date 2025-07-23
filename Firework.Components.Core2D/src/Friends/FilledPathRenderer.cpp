#include "FilledPathRenderer.h"

#include <EntityComponentSystem/EngineEvent.h>
#include <GL/Renderer.h>
#include <GL/Shader.h>
#include <Library/Math.h>

#include <Colored.vfAll.h>
#include <StencilPath.vfAll.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;

GeometryProgram FilledPathRenderer::stencilProgram = nullptr;
GeometryProgram FilledPathRenderer::drawProgram = nullptr;

StaticMesh FilledPathRenderer::unitSquare = nullptr;

bool FilledPathRenderer::renderInitialize()
{
    InternalEngineEvent::OnRenderShutdown += []
    {
        if (FilledPathRenderer::stencilProgram) [[likely]]
            FilledPathRenderer::stencilProgram = nullptr;
        if (FilledPathRenderer::drawProgram) [[likely]]
            FilledPathRenderer::drawProgram = nullptr;
        if (FilledPathRenderer::unitSquare) [[likely]]
            FilledPathRenderer::unitSquare = nullptr;
    };

    createShaderFromPrecompiled(FilledPathRenderer::stencilProgram, StencilPath);
    createShaderFromPrecompiled(FilledPathRenderer::drawProgram, Colored, std::array { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });

    float unitSquareVerts[] {
        -1.0f, -1.0f, 1.0f, // [0]
        -1.0f, 1.0f,  1.0f, // [1]
        1.0f,  1.0f,  1.0f, // [2]
        1.0f,  -1.0f, 1.0f  // [3]
    };
    uint16_t unitSquareInds[] { 2, 1, 0, 3, 2, 0 };
    FilledPathRenderer::unitSquare = StaticMesh(
        std::span(_asr(byte*, &unitSquareVerts), sizeof(unitSquareVerts)),
        VertexLayout(std::array { VertexDescriptor { .attribute = VertexAttributeName::Position, .type = VertexAttributeType::Float, .count = 3 } }), std::span(unitSquareInds));

    return FilledPathRenderer::stencilProgram && FilledPathRenderer::drawProgram && FilledPathRenderer::unitSquare;
}

FilledPathRenderer::FilledPathRenderer(const std::span<const FilledPathPoint> points, const std::span<const ssz> closedPathRanges)
{
    _fence_value_return(, points.size() < 3);
    _fence_value_return(, closedPathRanges.size() < 2);

    std::vector<FilledPathPoint> verts;
    verts.reserve(+sz(points.size()));
    verts.insert(verts.begin(), points.begin(), points.end());

    std::vector<uint16_t> inds;
    inds.reserve(+((sz(points.size()) - 2_uz) * 3_uz)); // Enough when one path of just lines, still minimizing unnecessary reallocations.

    for (auto boundBegIt = closedPathRanges.begin(); boundBegIt != --closedPathRanges.end(); ++boundBegIt)
    {
        auto boundEndIt = ++decltype(boundBegIt)(boundBegIt);
        for (u16 i = u16(*boundBegIt + 1_z); i < u16(*boundEndIt - 1_z); i++)
        {
            if (verts[+i].xCtrl == 0.0f) // Quadratic control point, defer.
                continue;

            inds.push_back(+u16(*boundBegIt));
            inds.push_back(+i);
            if (verts[+(i + 1_u16)].xCtrl != 0.0f)
                inds.push_back(+(i + 1_u16));
            else
                inds.push_back(+(i + 2_u16));
        }
        for (u16 i = u16(*boundBegIt); i < u16(*boundEndIt - 1_z); i++)
        {
            if (verts[+(i + 1_u16)].xCtrl != 0.0f) // Check quadratic control point.
                continue;

            inds.push_back(+i);
            inds.push_back(+(i + 1_u16));
            if (i + 2_u16 < u16(*boundEndIt))
                inds.push_back(+(i + 2_u16));
            else
                inds.push_back(+u16(*boundBegIt));
        }
    }

    this->fill = StaticMesh(std::span(_asr(byte*, verts.data()), verts.size() * sizeof(decltype(verts)::value_type)),
                            VertexLayout(std::array { VertexDescriptor { .attribute = VertexAttributeName::Position, .type = VertexAttributeType::Float, .count = 3 },
                                                      VertexDescriptor { .attribute = VertexAttributeName::TexCoord0, .type = VertexAttributeType::Float, .count = 2 } }),
                            inds);
}

bool FilledPathRenderer::submitDrawStencil(const ssz renderIndex, glm::mat4 shape, const bool forceHole) const
{
    _fence_value_return(false, !this->fill || !FilledPathRenderer::stencilProgram || !FilledPathRenderer::drawProgram);

    glm::mat4 shapeTransform = glm::translate(glm::mat4(1.0f), LinAlgConstants::forward * float(+renderIndex));
    shape = shapeTransform * shape;
    Renderer::setDrawTransform(shape);

    Renderer::setDrawStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0) |
                             (forceHole ? BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_REPLACE
                                        : BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_INVERT));
    Renderer::submitDraw(1, this->fill, FilledPathRenderer::stencilProgram, BGFX_STATE_DEPTH_TEST_LESS);

    return true;
}
bool FilledPathRenderer::submitDraw(const ssz renderIndex, glm::mat4 clip, const u8 whenStencil, const Color color)
{
    _fence_value_return(false, !FilledPathRenderer::unitSquare || !FilledPathRenderer::stencilProgram || !FilledPathRenderer::drawProgram);

    float colUniform[4] { float(color.r) / 255.0f, float(color.g) / 255.0f, float(color.b) / 255.0f, float(color.a) / 255.0f };
    FilledPathRenderer::drawProgram.setUniform("u_color", &colUniform);

    glm::mat4 clipTransform = glm::translate(glm::mat4(1.0f), LinAlgConstants::forward * float(+renderIndex));
    clip = clipTransform * clip;
    Renderer::setDrawTransform(clip);

    Renderer::setDrawStencil(BGFX_STENCIL_TEST_EQUAL | BGFX_STENCIL_FUNC_REF(+whenStencil) | BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
                             BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP);
    Renderer::submitDraw(1, FilledPathRenderer::unitSquare, FilledPathRenderer::drawProgram);

    return true;
}
