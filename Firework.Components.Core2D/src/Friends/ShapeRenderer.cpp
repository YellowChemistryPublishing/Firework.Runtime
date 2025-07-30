#include "ShapeRenderer.h"

#include <EntityComponentSystem/EngineEvent.h>
#include <GL/Renderer.h>
#include <GL/Shader.h>
#include <GL/Texture.h>
#include <Library/Math.h>

#include <Colored.vfAll.h>
#include <StencilPath.vfAll.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;

GeometryProgram ShapeRenderer::stencilProgram = nullptr;
GeometryProgram ShapeRenderer::drawProgram = nullptr;

StaticMesh ShapeRenderer::unitSquare = nullptr;

bool ShapeRenderer::renderInitialize()
{
    InternalEngineEvent::OnRenderShutdown += []
    {
        if (ShapeRenderer::stencilProgram) [[likely]]
            ShapeRenderer::stencilProgram = nullptr;
        if (ShapeRenderer::drawProgram) [[likely]]
            ShapeRenderer::drawProgram = nullptr;

        if (ShapeRenderer::unitSquare) [[likely]]
            ShapeRenderer::unitSquare = nullptr;
    };

    createShaderFromPrecompiled(ShapeRenderer::stencilProgram, StencilPath);
    createShaderFromPrecompiled(ShapeRenderer::drawProgram, Colored, std::array { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });

    float unitSquareVerts[] {
        -0.5f, -0.5f, 0.5f, // [0]
        -0.5f, 0.5f,  0.5f, // [1]
        0.5f,  0.5f,  0.5f, // [2]
        0.5f,  -0.5f, 0.5f  // [3]
    };
    uint16_t unitSquareInds[] { 2, 1, 0, 3, 2, 0 };
    ShapeRenderer::unitSquare = StaticMesh(
        std::span(_asr(byte*, &unitSquareVerts), sizeof(unitSquareVerts)),
        VertexLayout(std::array { VertexDescriptor { .attribute = VertexAttributeName::Position, .type = VertexAttributeType::Float, .count = 3 } }), std::span(unitSquareInds));

    return ShapeRenderer::stencilProgram && ShapeRenderer::drawProgram && ShapeRenderer::unitSquare;
}

ShapeRenderer::ShapeRenderer(const std::span<const ShapePoint> points, const std::span<const ssz> closedPathRanges)
{
    _fence_value_return(, points.size() < 3);
    _fence_value_return(, closedPathRanges.size() < 2);

    std::vector<ShapePoint> verts;
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

bool ShapeRenderer::submitDrawStencil(const float renderIndex, const glm::mat4 shape, const bool forceHole) const
{
    _fence_value_return(false, !this->fill || !ShapeRenderer::stencilProgram || !ShapeRenderer::drawProgram);

    glm::mat4 shapeTransform = glm::translate(glm::mat4(1.0f), LinAlgConstants::forward * renderIndex);
    shapeTransform *= shape;
    Renderer::setDrawTransform(shapeTransform);

    Renderer::setDrawStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0xFF) |
                             (forceHole ? BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_REPLACE
                                        : BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_INVERT));
    (void)Renderer::submitDraw(1, this->fill, ShapeRenderer::stencilProgram, BGFX_STATE_DEPTH_TEST_LESS);

    return true;
}
bool ShapeRenderer::submitDraw(const float renderIndex, const glm::mat4 clip, const u8 whenStencil, const Color color)
{
    _fence_value_return(false, !ShapeRenderer::unitSquare || !ShapeRenderer::stencilProgram || !ShapeRenderer::drawProgram);

    float colUniform[4] { float(color.r) / 255.0f, float(color.g) / 255.0f, float(color.b) / 255.0f, float(color.a) / 255.0f };
    (void)ShapeRenderer::drawProgram.setUniform("u_color", &colUniform);

    glm::mat4 clipTransform = glm::translate(glm::mat4(1.0f), LinAlgConstants::forward * renderIndex);
    clipTransform *= clip;
    Renderer::setDrawTransform(clip);

    Renderer::setDrawStencil(BGFX_STENCIL_TEST_EQUAL | BGFX_STENCIL_FUNC_REF(+whenStencil) | BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
                             BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP);
    (void)Renderer::submitDraw(1, ShapeRenderer::unitSquare, ShapeRenderer::drawProgram);

    return true;
}
