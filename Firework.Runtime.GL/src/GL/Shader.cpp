#include "Shader.h"

#include <bgfx/bgfx.h>
#include <cstdint>
#include <cstring>

#include <GL/Renderer.h>

using namespace Firework::GL;

GeometryProgramHandle GeometryProgramHandle::create(void* vertexShaderData, uint32_t vertexShaderDataSize, void* fragmentShaderData, uint32_t fragmentShaderDataSize,
                                                    const ShaderUniform* uniforms, size_t uniformsLength)
{
    GeometryProgramHandle ret;
    ret.internalHandle = bgfx::createProgram(bgfx::createShader(bgfx::copy(vertexShaderData, vertexShaderDataSize)),
                                             bgfx::createShader(bgfx::copy(fragmentShaderData, fragmentShaderDataSize)), true);
    for (size_t i = 0; i < uniformsLength; i++)
    {
        if (!ret.internalUniformHandles.count(uniforms[i].name))
            ret.internalUniformHandles.emplace(std::move(uniforms[i].name), new Uniform(uniforms[i].name, uniforms[i].type, uniforms[i].count));
    }
    return ret;
}
void GeometryProgramHandle::destroy()
{
    for (auto& [_, uniform] : this->internalUniformHandles) delete uniform;
    bgfx::destroy(this->internalHandle);
}

bool GeometryProgramHandle::setUniform(std::string_view name, const void* value)
{
    return this->setArrayUniform(name, value, 1_u16);
}
bool GeometryProgramHandle::setArrayUniform(std::string_view name, const void* value, u16 count)
{
    auto it = this->internalUniformHandles.find(name);
    _fence_value_return(false, it == this->internalUniformHandles.end());

    Renderer::setDrawArrayUniform(*it->second, value, count);
    return true;
}