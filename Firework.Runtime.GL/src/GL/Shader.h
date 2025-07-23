#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <cstring>
#include <map>
#include <module/sys>
#include <vector>

#include <GL/Uniform.h>

#define _concat(a, b) a##b
#define _recat(a, b) _concat(a, b)
#define getGeometryProgramArgsFromPrecompiledShaderName(shaderName, backend)                                       \
    std::span(_asr(const byte*, _recat(_recat(_recat(_concat(shader, shaderName), Vertex), backend), Data)),       \
              _recat(_recat(_recat(_recat(_concat(shader, shaderName), Vertex), backend), Data), _Size)),          \
        std::span(_asr(const byte*, _recat(_recat(_recat(_concat(shader, shaderName), Fragment), backend), Data)), \
                  _recat(_recat(_recat(_recat(_concat(shader, shaderName), Fragment), backend), Data), _Size))

#if _WIN32
#define createShaderFromPrecompiled(prog, shaderName, ...)                                                                      \
    switch (Renderer::rendererBackend())                                                                                        \
    {                                                                                                                           \
    case RendererBackend::Direct3D11:                                                                                           \
        prog = GeometryProgram(getGeometryProgramArgsFromPrecompiledShaderName(shaderName, d3d11) __VA_OPT__(, ) __VA_ARGS__);  \
        break;                                                                                                                  \
    case RendererBackend::Direct3D12:                                                                                           \
        prog = GeometryProgram(getGeometryProgramArgsFromPrecompiledShaderName(shaderName, d3d12) __VA_OPT__(, ) __VA_ARGS__);  \
        break;                                                                                                                  \
    case RendererBackend::OpenGL:                                                                                               \
        prog = GeometryProgram(getGeometryProgramArgsFromPrecompiledShaderName(shaderName, opengl) __VA_OPT__(, ) __VA_ARGS__); \
        break;                                                                                                                  \
    case RendererBackend::Vulkan:                                                                                               \
        prog = GeometryProgram(getGeometryProgramArgsFromPrecompiledShaderName(shaderName, vulkan) __VA_OPT__(, ) __VA_ARGS__); \
        break;                                                                                                                  \
    }
#else
#define createShaderFromPrecompiled(prog, shaderName, ...)                                                                      \
    switch (Renderer::rendererBackend())                                                                                        \
    {                                                                                                                           \
    case RendererBackend::OpenGL:                                                                                               \
        prog = GeometryProgram(getGeometryProgramArgsFromPrecompiledShaderName(shaderName, opengl) __VA_OPT__(, ) __VA_ARGS__); \
        break;                                                                                                                  \
    case RendererBackend::Vulkan:                                                                                               \
        prog = GeometryProgram(getGeometryProgramArgsFromPrecompiledShaderName(shaderName, vulkan) __VA_OPT__(, ) __VA_ARGS__); \
        break;                                                                                                                  \
    }
#endif

namespace Firework::GL
{
    class Renderer;

    struct ShaderUniform
    {
        std::string_view name;
        UniformType type;
        u16 count = 1;
    };

    class _fw_gl_api GeometryProgram final
    {
        bgfx::ProgramHandle internalHandle { .idx = bgfx::kInvalidHandle };
        std::map<std::string_view, sys::aligned_storage<Uniform>> internalUniformHandles;
    public:
        GeometryProgram(std::span<const byte> vertexShaderData, std::span<const byte> fragmentShaderData,
                        std::span<const ShaderUniform> uniforms = std::span<const ShaderUniform>());

        [[nodiscard]] bool setUniform(std::string_view name, const void* value);
        [[nodiscard]] bool setArrayUniform(std::string_view name, const void* value, u16 count);

        _fw_gl_common_handle_interface(GeometryProgram);

        friend void swap(GeometryProgram& a, GeometryProgram& b) noexcept
        {
            using std::swap;

            swap(a.internalHandle, b.internalHandle);
            swap(a.internalUniformHandles, b.internalUniformHandles);
        }
    };
} // namespace Firework::GL
