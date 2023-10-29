#include "Lighting.h"

#include <bgfx/bgfx.h>

#include <GL/Shader.h>

#include <PhongLit.vfAll.h>

using namespace Firework::Internal;
using namespace Firework::GL;

GeometryProgramHandle Lighting::lightingProgram;
constexpr static ShaderUniform lightingProgramUniforms[]
{
    { "u_normalMatrix", UniformType::Mat3 },
    { "u_directionalLights", UniformType::Mat4 },
    { "u_pointLights", UniformType::Mat4 },
    { "u_material", UniformType::Mat4 }
};

std::list<DirectionalLight> Lighting::dirLights;
std::list<PointLight> Lighting::pointLights;

GeometryProgramHandle Lighting::createProgramGeneric()
{
    switch (bgfx::getRendererType())
    {
    #if _WIN32
    case bgfx::RendererType::Direct3D9:
        return GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(PhongLit, d3d9), lightingProgramUniforms);
        break;
    case bgfx::RendererType::Direct3D11:
        return GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(PhongLit, d3d11), lightingProgramUniforms);
        break;
    case bgfx::RendererType::Direct3D12:
        return GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(PhongLit, d3d12), lightingProgramUniforms);
        break;
    #endif
    case bgfx::RendererType::OpenGL:
        return GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(PhongLit, opengl), lightingProgramUniforms);
        break;
    case bgfx::RendererType::Vulkan:
        return GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(PhongLit, vulkan), lightingProgramUniforms);
        break;
    default:
        throw "what";
    }
}
void Lighting::setLightingModel(LightingModel lm)
{
    Lighting::lightingProgram.destroy();
    switch (lm)
    {
    case LightingModel::Generic:
        Lighting::lightingProgram = Lighting::createProgramGeneric();
        break;
    case LightingModel::PhysicallyBased:
        throw "Unimplemented.";
        break;
    }
}