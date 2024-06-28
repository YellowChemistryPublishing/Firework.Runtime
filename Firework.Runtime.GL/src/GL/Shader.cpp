#include "Shader.h"

#include <bgfx/bgfx.h>
#include <cstdint>
#include <cstring>

#include <GL/Renderer.h>

using namespace Firework::GL;

GeometryProgramHandle GeometryProgramHandle::create
(
    void* vertexShaderData, uint32_t vertexShaderDataSize,
    void* fragmentShaderData, uint32_t fragmentShaderDataSize,
    const ShaderUniform* uniforms, size_t uniformsLength
)
{
    GeometryProgramHandle ret;
    char* vertData = new char[vertexShaderDataSize];
    memcpy(vertData, vertexShaderData, vertexShaderDataSize);
    char* fragData = new char[fragmentShaderDataSize];
    memcpy(fragData, fragmentShaderData, fragmentShaderDataSize);
    ret.internalHandle = bgfx::createProgram
    (
        bgfx::createShader(bgfx::makeRef(vertData, vertexShaderDataSize, [](void* data, void*) { delete[] static_cast<char*>(data); })),
        bgfx::createShader(bgfx::makeRef(fragData, fragmentShaderDataSize, [](void* data, void*) { delete[] static_cast<char*>(data); })),
        true
    );
    for (size_t i = 0; i < uniformsLength; i++)
    {
        if (!ret.internalUniformHandles.count(uniforms[i].name))
            ret.internalUniformHandles.emplace(uniforms[i].name, UniformHandle::createArray(uniforms[i].name, uniforms[i].type, uniforms[i].count));
    }
    return ret;
}
void GeometryProgramHandle::destroy()
{
    for (auto&[_, internalUniformHandle] : this->internalUniformHandles)
        internalUniformHandle.destroy();
    this->internalUniformHandles.clear();
    bgfx::destroy(this->internalHandle);
}

void GeometryProgramHandle::setUniform(const char* name, const void* value)
{
    auto it = this->internalUniformHandles.find(name);
    if (it != this->internalUniformHandles.end())
        Renderer::setDrawUniform(it->second, value);
}
void GeometryProgramHandle::setArrayUniform(const char* name, const void* value, uint16_t count)
{
    auto it = this->internalUniformHandles.find(name);
    if (it != this->internalUniformHandles.end())
        Renderer::setDrawArrayUniform(it->second, value, count);
}