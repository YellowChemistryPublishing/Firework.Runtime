#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <type_traits>

#define _fw_gl_common_mh_interface(T)                                                                 \
    T(std::nullptr_t) noexcept                                                                        \
    { }                                                                                               \
    T(const T&) = delete;                                                                             \
    T(T&& other) noexcept                                                                             \
    {                                                                                                 \
        swap(*this, other);                                                                           \
    }                                                                                                 \
    ~T();                                                                                             \
                                                                                                      \
    T& operator=(const T&) = delete;                                                                  \
    T& operator=(T&& other) noexcept                                                                  \
    {                                                                                                 \
        swap(*this, other);                                                                           \
        return *this;                                                                                 \
    }                                                                                                 \
                                                                                                      \
    operator bool() const noexcept                                                                    \
    {                                                                                                 \
        return bgfx::isValid(this->internalVertexBuffer) && bgfx::isValid(this->internalIndexBuffer); \
    }                                                                                                 \
                                                                                                      \
    friend void swap(T& a, T& b) noexcept                                                             \
    {                                                                                                 \
        using std::swap;                                                                              \
                                                                                                      \
        swap(a.internalVertexBuffer, b.internalVertexBuffer);                                         \
        swap(a.internalIndexBuffer, b.internalIndexBuffer);                                           \
    }                                                                                                 \
                                                                                                      \
    friend class Firework::GL::Renderer

namespace Firework::GL
{
    class Renderer;

    class StaticMesh;
    class DynamicMesh;

    enum class VertexAttributeName
    {
        Position = bgfx::Attrib::Position,
        Normal = bgfx::Attrib::Normal,
        Tangent = bgfx::Attrib::Tangent,
        Bitangent = bgfx::Attrib::Bitangent,
        Color0 = bgfx::Attrib::Color0,
        Color1 = bgfx::Attrib::Color1,
        Color2 = bgfx::Attrib::Color2,
        Color3 = bgfx::Attrib::Color3,
        Indices = bgfx::Attrib::Indices,
        Weight = bgfx::Attrib::Weight,
        TexCoord0 = bgfx::Attrib::TexCoord0,
        TexCoord1 = bgfx::Attrib::TexCoord1,
        TexCoord2 = bgfx::Attrib::TexCoord2,
        TexCoord3 = bgfx::Attrib::TexCoord3,
        TexCoord4 = bgfx::Attrib::TexCoord4,
        TexCoord5 = bgfx::Attrib::TexCoord5,
        TexCoord6 = bgfx::Attrib::TexCoord6,
        TexCoord7 = bgfx::Attrib::TexCoord7
    };
    enum class VertexAttributeType
    {
        Uint8 = bgfx::AttribType::Uint8,
        Uint10 = bgfx::AttribType::Uint10,
        Int16 = bgfx::AttribType::Int16,
        Half = bgfx::AttribType::Half,
        Float = bgfx::AttribType::Float
    };
    struct VertexDescriptor
    {
        VertexAttributeName attribute;
        VertexAttributeType type;
        u8 count;
        bool normalize = false;
    };

    struct _fw_gl_api VertexLayout final
    {
        VertexLayout(std::span<const VertexDescriptor> descriptors);

        friend class Firework::GL::Renderer;
        friend class Firework::GL::StaticMesh;
        friend class Firework::GL::DynamicMesh;
    private:
        bgfx::VertexLayout internalLayout;
    };

    class _fw_gl_api StaticMesh final
    {
        bgfx::VertexBufferHandle internalVertexBuffer { .idx = bgfx::kInvalidHandle };
        bgfx::IndexBufferHandle internalIndexBuffer { .idx = bgfx::kInvalidHandle };
    public:
        StaticMesh(std::span<const byte> vertexData, const VertexLayout& vl, std::span<const uint16_t> indexData);

        _fw_gl_common_mh_interface(StaticMesh);
    };
    class _fw_gl_api DynamicMesh final
    {
        bgfx::DynamicVertexBufferHandle internalVertexBuffer { .idx = bgfx::kInvalidHandle };
        bgfx::DynamicIndexBufferHandle internalIndexBuffer { .idx = bgfx::kInvalidHandle };
    public:
        DynamicMesh(std::span<const byte> vertexData, const VertexLayout& vl, std::span<const uint16_t> indexData);

        [[nodiscard]] bool update(std::span<const byte> vertexData, std::span<const uint16_t> indexData, u32 fromVertex = 0, u32 fromIndex = 0);

        _fw_gl_common_mh_interface(DynamicMesh);
    };
} // namespace Firework::GL