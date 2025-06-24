#include "Mask.h"

#include <Components/RectTransform.h>
#include <Core/CoreEngine.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <GL/Renderer.h>

#include <NoOp.vfAll.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;

GL::StaticMeshHandle Mask::unitSquare;
GL::GeometryProgramHandle Mask::program;

uint32_t Mask::currentRenderMaskValue = 0;

void Mask::renderInitialize()
{
    CoreEngine::queueRenderJobForFrame([]
    {
        switch (Renderer::rendererBackend())
        {
        #if _WIN32
        case RendererBackend::Direct3D11:
            Mask::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(NoOp, d3d11));
            break;
        case RendererBackend::Direct3D12:
            Mask::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(NoOp, d3d12));
            break;
        #endif
        case RendererBackend::OpenGL:
            Mask::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(NoOp, opengl));
            break;
        case RendererBackend::Vulkan:
            Mask::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(NoOp, vulkan));
            break;
        default:
            // TODO: Implement.
            throw "unimplemented";
        }
        
        sysm::vector3 unitSquareVerts[]
        {
            { -1.0f, -1.0f, 1.0f },
            { -1.0f, 1.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f },
            { 1.0f, -1.0f, 1.0f }
        };
        uint16_t unitSquareInds[]
        {
            2, 1, 0,
            3, 2, 0
        };
        Mask::unitSquare = StaticMeshHandle::create
        (
            unitSquareVerts, sizeof(unitSquareVerts),
            VertexLayout::create({ VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 } }),
            unitSquareInds, sizeof(unitSquareInds)
        );
    }, true);
    InternalEngineEvent::OnRenderShutdown += []
    {
        Mask::unitSquare.destroy();
        Mask::program.destroy();
    };
}
void Mask::renderFirstPass(bgfx::ViewId, void*)
{
    Renderer::setDrawStencil(BGFX_STENCIL_TEST_EQUAL | BGFX_STENCIL_FUNC_REF(Mask::currentRenderMaskValue) | BGFX_STENCIL_FUNC_RMASK(0xff) | BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP);
}
void Mask::renderOffload()
{
    CoreEngine::queueRenderJobForFrame([t = renderTransformFromRectTransform(this->rectTransform())]
    {
        Renderer::setDrawStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0xff) | BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_INCR);
        Renderer::setDrawTransform(t);
        Renderer::submitDraw
        (
            1, Mask::unitSquare, Mask::program,
            BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_BLEND_ALPHA
        );
        
        if (Mask::currentRenderMaskValue == 0)
            Renderer::addDrawPassIntercept(Mask::renderFirstPass);
        ++Mask::currentRenderMaskValue;
    }, false);
}
void Mask::lateRenderOffload()
{
    CoreEngine::queueRenderJobForFrame([t = renderTransformFromRectTransform(this->rectTransform())]
    {
        --Mask::currentRenderMaskValue;
        if (Mask::currentRenderMaskValue == 0)
            Renderer::removeDrawPassIntercept(Mask::renderFirstPass);

        //Renderer::setDrawStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0xff) | BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_DECR);
        //Renderer::setDrawTransform(t);
        //Renderer::submitDraw
        //(
        //    1, Mask::unitSquare, Mask::program,
        //    BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_BLEND_ALPHA
        //);
    }, false);
}