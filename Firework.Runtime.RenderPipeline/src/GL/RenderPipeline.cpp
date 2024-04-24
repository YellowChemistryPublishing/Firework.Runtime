#include "RenderPipeline.h"

#include <GL/Light.h>
#include <GL/Renderer.h>

#include <Generic.vfAll.h>

using namespace Firework;
using namespace Firework::GL;

static GeometryProgramHandle genericProgram;

void RenderPipeline::init()
{
    switch (Renderer::rendererBackend())
    {
    #if _WIN32
    case RendererBackend::Direct3D11:
        genericProgram = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Generic, d3d11), { ShaderUniform { .name = "u_ambientData", .type = UniformType::Vec4, .count = 1 } });
        break;
    case RendererBackend::Direct3D12:
        genericProgram = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Generic, d3d12), { ShaderUniform { .name = "u_ambientData", .type = UniformType::Vec4, .count = 1 } });
        break;
    #endif
    case RendererBackend::OpenGL:
        genericProgram = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Generic, opengl), { ShaderUniform { .name = "u_ambientData", .type = UniformType::Vec4, .count = 1 } });
        break;
    case RendererBackend::Vulkan:
        genericProgram = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Generic, vulkan), { ShaderUniform { .name = "u_ambientData", .type = UniformType::Vec4, .count = 1 } });
        break;
    default:
        throw "unimplemented";
    }

    DirectionalLightManager::ctor();
}
void RenderPipeline::deinit()
{
    DirectionalLightManager::dtor();

    genericProgram.destroy();
}

void RenderPipeline::drawMesh(GL::StaticMeshHandle mesh)
{
    float ambient[4] { 0.015f, 0.0f, 0.0f, 0.0f };
    genericProgram.setUniform("u_ambientData", ambient);
    Renderer::setDrawTexture(0, DirectionalLightManager::directionalLightsAcc, DirectionalLightManager::directionalLightsSampler);
    Renderer::submitDraw(0, mesh, genericProgram);
}