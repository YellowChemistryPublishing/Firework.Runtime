#include "Shader.h"

#include <bgfx/bgfx.h>
#include <cstdint>
#include <cstring>

#include <GL/Renderer.h>

using namespace Firework::GL;

GeometryProgram::GeometryProgram(const std::span<const byte> vertexShaderData, const std::span<const byte> fragmentShaderData, const std::span<const ShaderUniform> uniforms)
{
    sys::sc_act rollback = [&]
    {
        for (auto& [_, uniform] : this->internalUniformHandles) std::destroy_at(_asr(Uniform*, uniform.data()));
    };

    for (const auto& shaderUniform : uniforms)
    {
        if (!this->internalUniformHandles.count(shaderUniform.name))
        {
            std::construct_at(this->internalUniformHandles.emplace(std::move(shaderUniform.name), sys::aligned_storage<Uniform>()).first->second.data(), shaderUniform.name,
                              shaderUniform.type, shaderUniform.count);
        }
    }

    this->internalHandle = bgfx::createProgram(bgfx::createShader(bgfx::copy(vertexShaderData.data(), vertexShaderData.size_bytes())),
                                               bgfx::createShader(bgfx::copy(fragmentShaderData.data(), fragmentShaderData.size_bytes())), true);

    rollback.release();
}
GeometryProgram::~GeometryProgram()
{
    for (auto& [_, uniform] : this->internalUniformHandles) std::destroy_at(_asr(Uniform*, uniform.data()));
    if (bgfx::isValid(this->internalHandle))
        bgfx::destroy(this->internalHandle);
}

bool GeometryProgram::setUniform(const std::string_view name, const void* const value)
{
    return this->setArrayUniform(name, value, 1_u16);
}
bool GeometryProgram::setArrayUniform(const std::string_view name, const void* const value, const u16 count)
{
    auto it = this->internalUniformHandles.find(name);
    _fence_value_return(false, it == this->internalUniformHandles.end());

    _fence_value_return(false, !Renderer::setDrawArrayUniform(*_asr(Uniform*, it->second.data()), value, count));

    return true;
}
