#include "ShapeRenderer.h"
#include "bgfx/defines.h"

#include <EntityComponentSystem/EngineEvent.h>
#include <GL/Renderer.h>
#include <GL/Shader.h>
#include <GL/Texture.h>
#include <Library/Math.h>

#include <ShapeOutline.vfAll.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;

GeometryProgram ShapeRenderer::drawProgram = nullptr;

StaticMesh ShapeRenderer::unitSquare = nullptr;

bool ShapeRenderer::renderInitialize()
{
    InternalEngineEvent::OnRenderShutdown += []
    {
        if (ShapeRenderer::drawProgram) [[likely]]
            ShapeRenderer::drawProgram = nullptr;
        if (ShapeRenderer::unitSquare) [[likely]]
            ShapeRenderer::unitSquare = nullptr;
    };

    createShaderFromPrecompiled(ShapeRenderer::drawProgram, ShapeOutline,
                                std::array { ShaderUniform { .name = "u_params", .type = UniformType::Vec4 }, ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });

    float unitSquareVerts[] {
        -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, // [0]
        -0.5f, 0.5f,  1.0f, 0.0f, 1.0f, // [1]
        0.5f,  0.5f,  1.0f, 0.0f, 1.0f, // [2]
        0.5f,  -0.5f, 1.0f, 0.0f, 1.0f  // [3]
    };
    uint16_t unitSquareInds[] { 2, 1, 0, 3, 2, 0 };
    ShapeRenderer::unitSquare =
        StaticMesh(std::span(_asr(byte*, &unitSquareVerts), sizeof(unitSquareVerts)),
                   VertexLayout(std::array { VertexDescriptor { .attribute = VertexAttributeName::Position, .type = VertexAttributeType::Float, .count = 3 },
                                             VertexDescriptor { .attribute = VertexAttributeName::TexCoord0, .type = VertexAttributeType::Float, .count = 2 } }),
                   std::span(unitSquareInds));

    return ShapeRenderer::drawProgram && ShapeRenderer::unitSquare;
}

ShapeRenderer::ShapeRenderer(const std::span<const ShapePoint> points, const std::span<const uint16_t> inds, const u32 curveIndsBegin) :
    curvePointsSize(points.size()), curveIndsBegin(curveIndsBegin), curveIndsEnd(inds.size())
{
    _fence_value_return(, points.size() < 3);
    _fence_value_return(, inds.size() < 3);

    this->fill = StaticMesh(std::span(_asr(const byte*, points.data()), points.size() * sizeof(decltype(points)::value_type)),
                            VertexLayout(std::array { VertexDescriptor { .attribute = VertexAttributeName::Position, .type = VertexAttributeType::Float, .count = 3 },
                                                      VertexDescriptor { .attribute = VertexAttributeName::TexCoord0, .type = VertexAttributeType::Float, .count = 2 } }),
                            inds);
}

bool ShapeRenderer::submitDrawStencil(const float renderIndex, const glm::mat4 shape, const FillRule fillRule) const
{
    _fence_value_return(false, !this->fill || !ShapeRenderer::drawProgram);

    float colUniform[4] { 0.0f, 0.0f, 0.0f, 1.0f };

    glm::mat4 shapeTransform = glm::translate(glm::mat4(1.0f), LinAlgConstants::forward * renderIndex);
    shapeTransform *= shape;

    float paramsUniform[4] { 0.0f, 0.0f, 0.0f, 0.0f };

    _push_nowarn_gcc(_clWarn_gcc_c_cast);
    _push_nowarn_clang(_clWarn_clang_c_cast);

    (void)ShapeRenderer::drawProgram.setUniform("u_params", &paramsUniform);
    Renderer::setDrawTransform(shapeTransform);
    if (fillRule == FillRule::EvenOdd)
        Renderer::setDrawStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP |
                                 BGFX_STENCIL_OP_PASS_Z_INVERT);
    else
        Renderer::setDrawStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP |
                                     BGFX_STENCIL_OP_PASS_Z_INCR,
                                 BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP |
                                     BGFX_STENCIL_OP_PASS_Z_DECR);
    (void)Renderer::submitDraw(1, this->fill, ShapeRenderer::drawProgram,
                               BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_MSAA | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_ZERO));

    (void)ShapeRenderer::drawProgram.setUniform("u_color", &colUniform);
    paramsUniform[0] = 1.0f;
    (void)ShapeRenderer::drawProgram.setUniform("u_params", &paramsUniform);
    Renderer::setDrawTransform(shapeTransform);
    Renderer::setDrawStencil(BGFX_STENCIL_TEST_NOTEQUAL | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP |
                             BGFX_STENCIL_OP_PASS_Z_KEEP);
    (void)Renderer::submitDraw(1, this->fill, ShapeRenderer::drawProgram,
                               BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_MSAA | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE));

    (void)ShapeRenderer::drawProgram.setUniform("u_color", &colUniform);
    paramsUniform[0] = 2.0f;
    (void)ShapeRenderer::drawProgram.setUniform("u_params", &paramsUniform);
    Renderer::setDrawTransform(shapeTransform);
    Renderer::setDrawStencil(BGFX_STENCIL_TEST_EQUAL | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP |
                             BGFX_STENCIL_OP_PASS_Z_INCR);
    (void)Renderer::submitDraw(1, this->fill, ShapeRenderer::drawProgram,
                               BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_MSAA | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD) |
                                   BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE));

    _pop_nowarn_clang();
    _pop_nowarn_gcc();

    return true;
}
bool ShapeRenderer::submitDrawCover(const float renderIndex, const glm::mat4 clip, const u8 refZero, const Color color, const u64 blendState, const u32 stencilTest)
{
    _fence_value_return(false, !ShapeRenderer::unitSquare || !ShapeRenderer::drawProgram);

    float colUniform[4] { float(color.r) / 255.0f, float(color.g) / 255.0f, float(color.b) / 255.0f, float(color.a) / 255.0f };
    (void)ShapeRenderer::drawProgram.setUniform("u_color", &colUniform);

    float paramsUniform[4] { 1.0f, 0.0f, 0.0f, 0.0f };
    (void)ShapeRenderer::drawProgram.setUniform("u_params", &paramsUniform);

    glm::mat4 clipTransform = glm::translate(glm::mat4(1.0f), LinAlgConstants::forward * renderIndex);
    clipTransform *= clip;
    Renderer::setDrawTransform(clipTransform);

    _push_nowarn_gcc(_clWarn_gcc_c_cast);
    _push_nowarn_clang(_clWarn_clang_c_cast);
    if (stencilTest)
        Renderer::setDrawStencil(+stencilTest | BGFX_STENCIL_FUNC_REF(+refZero) | BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_Z_KEEP);
    (void)Renderer::submitDraw(1, ShapeRenderer::unitSquare, ShapeRenderer::drawProgram, BGFX_STATE_CULL_CW | +blendState);
    _pop_nowarn_clang();
    _pop_nowarn_gcc();

    return true;
}
