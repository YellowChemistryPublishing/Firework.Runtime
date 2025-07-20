#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <cstring>
#include <map>
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

namespace Firework
{
    namespace GL
    {
        class Renderer;

        struct ShaderUniform
        {
            const char* name;
            UniformType type;
            uint16_t count = 1;
        };

        class __firework_gl_api GeometryProgramHandle final
        {
            struct Comp
            {
                bool operator()(const char* lhs, const char* rhs) const
                {
                    return strcmp(lhs, rhs) < 0;
                }
            };

            bgfx::ProgramHandle internalHandle { .idx = bgfx::kInvalidHandle };
            std::map<const char*, UniformHandle, Comp> internalUniformHandles;

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

            void setUniform(const char* name, const void* value);
            void setArrayUniform(const char* name, const void* value, uint16_t count);

            friend class Firework::GL::Renderer;
        };
    } // namespace GL
} // namespace Firework