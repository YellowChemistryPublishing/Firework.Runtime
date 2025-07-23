#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <cstring>
#include <map>
#include <module/sys>
#include <vector>

#include <GL/Uniform.h>

#define __concat(a, b) a##b
#define __recat(a, b) __concat(a, b)
#define getGeometryProgramArgsFromPrecompiledShaderName(shaderName, backend)                                       \
    (void*)__recat(__recat(__recat(__concat(shader, shaderName), Vertex), backend), Data),                         \
        (uint32_t)__recat(__recat(__recat(__recat(__concat(shader, shaderName), Vertex), backend), Data), _Size),  \
        (void*)__recat(__recat(__recat(__concat(shader, shaderName), Fragment), backend), Data),                   \
        (uint32_t)__recat(__recat(__recat(__recat(__concat(shader, shaderName), Fragment), backend), Data), _Size)

#if _WIN32
#define createShaderFromPrecompiled(prog, shaderName, ...)                                                                                    \
    switch (Renderer::rendererBackend())                                                                                                      \
    {                                                                                                                                         \
    case RendererBackend::Direct3D11:                                                                                                         \
        prog = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(shaderName, d3d11) __VA_OPT__(, ) __VA_ARGS__);  \
        break;                                                                                                                                \
    case RendererBackend::Direct3D12:                                                                                                         \
        prog = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(shaderName, d3d12) __VA_OPT__(, ) __VA_ARGS__);  \
        break;                                                                                                                                \
    case RendererBackend::OpenGL:                                                                                                             \
        prog = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(shaderName, opengl) __VA_OPT__(, ) __VA_ARGS__); \
        break;                                                                                                                                \
    case RendererBackend::Vulkan:                                                                                                             \
        prog = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(shaderName, vulkan) __VA_OPT__(, ) __VA_ARGS__); \
        break;                                                                                                                                \
    }
#else
#define createShaderFromPrecompiled(prog, shaderName, ...)                                                                                    \
    switch (Renderer::rendererBackend())                                                                                                      \
    {                                                                                                                                         \
    case RendererBackend::OpenGL:                                                                                                             \
        prog = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(shaderName, opengl) __VA_OPT__(, ) __VA_ARGS__); \
        break;                                                                                                                                \
    case RendererBackend::Vulkan:                                                                                                             \
        prog = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(shaderName, vulkan) __VA_OPT__(, ) __VA_ARGS__); \
        break;                                                                                                                                \
    }
#endif

namespace Firework::GL
{
    class Renderer;

    struct ShaderUniform
    {
        std::string_view name;
        UniformType type;
        uint16_t count = 1;
    };

    class _fw_gl_api GeometryProgramHandle final
    {
        bgfx::ProgramHandle internalHandle { .idx = bgfx::kInvalidHandle };
        std::map<std::string_view, sys::aligned_storage<Uniform>> internalUniformHandles;

        static GeometryProgramHandle create(void* vertexShaderData, uint32_t vertexShaderDataSize, void* fragmentShaderData, uint32_t fragmentShaderDataSize,
                                            const ShaderUniform* uniforms, size_t uniformsLength);
    public:
        template <size_t N>
        inline static GeometryProgramHandle create(void* vertexShaderData, uint32_t vertexShaderDataSize, void* fragmentShaderData, uint32_t fragmentShaderDataSize,
                                                   const ShaderUniform (&uniforms)[N])
        {
            return GeometryProgramHandle::create(vertexShaderData, vertexShaderDataSize, fragmentShaderData, fragmentShaderDataSize, uniforms, N);
        }
        inline static GeometryProgramHandle create(void* vertexShaderData, uint32_t vertexShaderDataSize, void* fragmentShaderData, uint32_t fragmentShaderDataSize)
        {
            return GeometryProgramHandle::create(vertexShaderData, vertexShaderDataSize, fragmentShaderData, fragmentShaderDataSize, nullptr, 0);
        }
        void destroy();

        inline operator bool()
        {
            return bgfx::isValid(this->internalHandle);
        }

        [[nodiscard]] bool setUniform(std::string_view name, const void* value);
        [[nodiscard]] bool setArrayUniform(std::string_view name, const void* value, u16 count);

        friend class Firework::GL::Renderer;
    };
} // namespace Firework::GL
