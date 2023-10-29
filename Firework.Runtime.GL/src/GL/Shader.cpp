#include "Shader.h"

#include <bgfx/bgfx.h>
#include <cstdint>
#include <cstring>

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
        bgfx::createShader(bgfx::makeRef(vertData, vertexShaderDataSize, [](void* data, void*) { delete[] (char*)data; })),
        bgfx::createShader(bgfx::makeRef(fragData, fragmentShaderDataSize, [](void* data, void*) { delete[] (char*)data; })),
        true
    );
    for (size_t i = 0; i < uniformsLength; i++)
    {
        if (!ret.internalUniformHandles.count(uniforms[i].name))
            ret.internalUniformHandles.emplace(uniforms[i].name, bgfx::createUniform(uniforms[i].name, (bgfx::UniformType::Enum)uniforms[i].type, uniforms[i].count));
    }
    return ret;
}
void GeometryProgramHandle::destroy()
{
    for (auto&[_, internalUniformHandle] : this->internalUniformHandles)
        bgfx::destroy(internalUniformHandle);
    this->internalUniformHandles.clear();
    bgfx::destroy(this->internalHandle);
}

void GeometryProgramHandle::setUniform(const char* name, const void* value)
{
    auto it = this->internalUniformHandles.find(name);
    if (it != this->internalUniformHandles.end())
        bgfx::setUniform(it->second, value);
}
void GeometryProgramHandle::setUniform(const char* name, const void* value, uint16_t count)
{
    auto it = this->internalUniformHandles.find(name);
    if (it != this->internalUniformHandles.end())
        bgfx::setUniform(it->second, value, count);
}