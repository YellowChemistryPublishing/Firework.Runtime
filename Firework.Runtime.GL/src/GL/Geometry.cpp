#include "Geometry.h"

#include <cstring>

using namespace Firework::GL;

VertexLayout::VertexLayout(const std::span<const VertexDescriptor> descriptors)
{
    this->internalLayout.begin();
    for (const VertexDescriptor& desc : descriptors)
        this->internalLayout.add(_as(bgfx::Attrib::Enum, desc.attribute), +desc.count, _as(bgfx::AttribType::Enum, desc.type), desc.normalize);
    this->internalLayout.end();
}

#define _fw_gl_common_mh_ctor_dtor(T, createVertFn, createIndFn)                                                             \
    T::T(std::span<const byte> vertexData, const VertexLayout& vl, std::span<const uint16_t> indexData) :                    \
        internalVertexBuffer(createVertFn(bgfx::copy(vertexData.data(), +u32(vertexData.size_bytes())), vl.internalLayout)), \
        internalIndexBuffer(createIndFn(bgfx::copy(indexData.data(), +u32(indexData.size_bytes()))))                         \
    { }                                                                                                                      \
    T::~T()                                                                                                                  \
    {                                                                                                                        \
        if (bgfx::isValid(this->internalVertexBuffer))                                                                       \
            bgfx::destroy(this->internalVertexBuffer);                                                                       \
        if (bgfx::isValid(this->internalIndexBuffer))                                                                        \
            bgfx::destroy(this->internalIndexBuffer);                                                                        \
    }

_fw_gl_common_mh_ctor_dtor(StaticMesh, bgfx::createVertexBuffer, bgfx::createIndexBuffer);

bool DynamicMesh::update(const std::span<const byte> vertexData, const std::span<const uint16_t> indexData, const u32 fromVertex, const u32 fromIndex)
{
    _fence_value_return(false, !bgfx::isValid(this->internalVertexBuffer) || !bgfx::isValid(this->internalIndexBuffer));

    bgfx::update(this->internalVertexBuffer, +fromVertex, bgfx::copy(vertexData.data(), +u32(vertexData.size_bytes())));
    bgfx::update(this->internalIndexBuffer, +fromIndex, bgfx::copy(indexData.data(), +u32(indexData.size_bytes())));
    return true;
}

_fw_gl_common_mh_ctor_dtor(DynamicMesh, bgfx::createDynamicVertexBuffer, bgfx::createDynamicIndexBuffer);
