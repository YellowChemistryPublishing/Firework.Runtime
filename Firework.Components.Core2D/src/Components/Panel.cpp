#include "Panel.h"

#include <Mathematics.h>
#include <Core/CoreEngine.h>
#include <Components/RectTransform.h>
#include <EntityComponentSystem/EngineEvent.h>

#include <Panel.vfAll.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::Mathematics;
using namespace Firework::GL;

StaticMeshHandle Panel::unitSquare;
GeometryProgramHandle Panel::program;

void Panel::renderInitialize()
{
    CoreEngine::queueRenderJobForFrame([]
    {
        switch (Renderer::rendererBackend())
        {
        #if _WIN32
        case RendererBackend::Direct3D11:
            Panel::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Panel, d3d11), { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
            break;
        case RendererBackend::Direct3D12:
            Panel::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Panel, d3d12), { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
            break;
        #endif
        case RendererBackend::OpenGL:
            Panel::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Panel, opengl), { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
            break;
        case RendererBackend::Vulkan:
            Panel::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Panel, vulkan), { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
            break;
        default:
            // TODO: Implement.
            throw "unimplemented";
        }
        
        Vector3 unitSquareVerts[]
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
        Panel::unitSquare = StaticMeshHandle::create
        (
            unitSquareVerts, sizeof(unitSquareVerts),
            VertexLayout::create({ VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 } }),
            unitSquareInds, sizeof(unitSquareInds)
        );
    }, true);
    InternalEngineEvent::OnRenderShutdown += []
    {
        Panel::unitSquare.destroy();
        Panel::program.destroy();
    };
}
void Panel::renderOffload()
{
    CoreEngine::queueRenderJobForFrame([t = renderTransformFromRectTransform(this->rectTransform()), color = this->_color]
    {
        float col[4] { (float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, (float)color.a / 255.0f };
        program.setUniform("u_color", col);
        Renderer::setDrawTransform(t);
        Renderer::submitDraw
        (
            1, Panel::unitSquare, Panel::program,
            BGFX_STATE_CULL_CW |
            (color.r == 0 ? 0 : BGFX_STATE_WRITE_R) |
            (color.g == 0 ? 0 : BGFX_STATE_WRITE_G) |
            (color.b == 0 ? 0 : BGFX_STATE_WRITE_B) |
            (color.a == 0 ? 0 : BGFX_STATE_WRITE_A)
        );
    }, false);
}