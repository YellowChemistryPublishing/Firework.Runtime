#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>
#include <type_traits>

namespace Firework
{
    namespace GL
    {
        class Renderer;

        class StaticMeshHandle;
        class DynamicMeshHandle;

        struct VertexDescriptor
        {
            bgfx::Attrib::Enum attribute;
            bgfx::AttribType::Enum type;
            uint8_t count;
            bool normalize = false;
        };
        class __firework_gl_api VertexLayout final
        {
            bgfx::VertexLayout internalLayout;
        public:
            template <size_t N>
            inline static VertexLayout create(const VertexDescriptor (&descriptors)[N])
            {
                return VertexLayout::create(descriptors, N);
            };
            static VertexLayout create(const VertexDescriptor* descriptors, size_t descriptorsLength);

            friend class Firework::GL::Renderer;
            friend class Firework::GL::StaticMeshHandle;
            friend class Firework::GL::DynamicMeshHandle;
        };

        class __firework_gl_api StaticMeshHandle final
        {
            bgfx::VertexBufferHandle internalVertexBuffer;
            bgfx::IndexBufferHandle internalIndexBuffer;
        public:
            static StaticMeshHandle create(const void* vertexData, uint32_t vertexDataSize, VertexLayout vl, const uint16_t* indexData, uint32_t indexDataSize);
            void destroy();

            friend class Firework::GL::Renderer;
        };
        class __firework_gl_api DynamicMeshHandle final
        {
            bgfx::DynamicVertexBufferHandle internalVertexBuffer;
            bgfx::DynamicIndexBufferHandle internalIndexBuffer;
        public:
            inline DynamicMeshHandle() = default;
            inline DynamicMeshHandle(std::nullptr_t) : internalVertexBuffer(BGFX_INVALID_HANDLE), internalIndexBuffer(BGFX_INVALID_HANDLE)
            { }
            
            static DynamicMeshHandle create(const void* vertexData, uint32_t vertexDataSize, VertexLayout vl, const uint16_t* indexData, uint32_t indexDataSize);
            void update(const void* vertexData, uint32_t vertexDataSize, const uint16_t* indexData, uint32_t indexDataSize, uint32_t fromVertex = 0, uint32_t fromIndex = 0);
            void destroy();

            inline operator bool () const
            {
                return bgfx::isValid(this->internalVertexBuffer) && bgfx::isValid(this->internalIndexBuffer);
            }
            inline DynamicMeshHandle& operator=(const DynamicMeshHandle& other) = default;
            inline DynamicMeshHandle& operator=(std::nullptr_t)
            {
                this->internalVertexBuffer = BGFX_INVALID_HANDLE;
                this->internalIndexBuffer = BGFX_INVALID_HANDLE;
                return *this;
            }

            friend class Firework::GL::Renderer;
        };
    }
}