#include "FilledPathRenderer.h"

#include <EntityComponentSystem/EngineEvent.h>
#include <GL/Renderer.h>

#include <Path.vfAll.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;

GeometryProgramHandle FilledPathRenderer::program;
StaticMeshHandle FilledPathRenderer::unitSquare;

bool FilledPathRenderer::renderInitialize()
{
    InternalEngineEvent::OnRenderShutdown += []
    {
        if (FilledPathRenderer::program) [[likely]]
            FilledPathRenderer::program.destroy();
        if (FilledPathRenderer::unitSquare) [[likely]]
            FilledPathRenderer::unitSquare.destroy();
    };

    switch (Renderer::rendererBackend())
    {
#if _WIN32
    case RendererBackend::Direct3D11:
        FilledPathRenderer::program =
            GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Path, d3d11), { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
        break;
    case RendererBackend::Direct3D12:
        FilledPathRenderer::program =
            GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Path, d3d12), { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
        break;
#endif
    case RendererBackend::OpenGL:
        FilledPathRenderer::program =
            GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Path, opengl), { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
        break;
    case RendererBackend::Vulkan:
        FilledPathRenderer::program =
            GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Path, vulkan), { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
        break;
    default:;
    }

    float unitSquareVerts[] {
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // [0]
        -1.0f, 1.0f,  1.0f, 0.0f, 0.0f, // [1]
        1.0f,  1.0f,  1.0f, 0.0f, 0.0f, // [2]
        1.0f,  -1.0f, 1.0f, 0.0f, 0.0f  // [3]
    };
    uint16_t unitSquareInds[] { 2, 1, 0, 3, 2, 0 };
    FilledPathRenderer::unitSquare =
        StaticMeshHandle::create(unitSquareVerts, sizeof(unitSquareVerts),
                                 VertexLayout::create({ VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 },
                                                        VertexDescriptor { .attribute = bgfx::Attrib::TexCoord0, .type = bgfx::AttribType::Float, .count = 2 } }),
                                 unitSquareInds, sizeof(unitSquareInds));

    return FilledPathRenderer::program && FilledPathRenderer::unitSquare;
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

    this->fill = StaticMeshHandle::create(std::data(verts), +u32(verts.size() * sizeof(decltype(verts)::value_type)),
                                          VertexLayout::create({ VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 },
                                                                 VertexDescriptor { .attribute = bgfx::Attrib::TexCoord0, .type = bgfx::AttribType::Float, .count = 2 } }),
                                          inds.data(), +u32(inds.size() * sizeof(decltype(inds)::value_type)));
}

bool FilledPathRenderer::submitDrawStencil(const ssz renderIndex, RenderTransform shape, const bool forceHole)
{
    _fence_value_return(false, !this->fill || !FilledPathRenderer::program);

    shape.translate(sysm::vector3::forward * float(+renderIndex));
    Renderer::setDrawTransform(shape);

    Renderer::setDrawStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0) |
                             (forceHole ? BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_REPLACE
                                        : BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_INVERT));
    Renderer::submitDraw(1, this->fill, FilledPathRenderer::program, BGFX_STATE_DEPTH_TEST_LESS);

    return true;
}
bool FilledPathRenderer::submitDraw(const ssz renderIndex, RenderTransform clip, const u8 whenStencil)
{
    _fence_value_return(false, !FilledPathRenderer::unitSquare || !FilledPathRenderer::program);

    float col[4] { 1.0f, 1.0f, 1.0f, 1.0f };
    FilledPathRenderer::program.setUniform("u_color", &col);

    clip.translate(sysm::vector3::forward * float(+renderIndex));
    Renderer::setDrawTransform(clip);

    Renderer::setDrawStencil(BGFX_STENCIL_TEST_EQUAL | BGFX_STENCIL_FUNC_REF(+whenStencil) | BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
                             BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP);
    Renderer::submitDraw(1, FilledPathRenderer::unitSquare, FilledPathRenderer::program);

    return true;
}
