#include "Geometry.h"

#include <cstring>

using namespace Firework::GL;

VertexLayout VertexLayout::create(const VertexDescriptor* descriptors, size_t descriptorsLength)
{
    VertexLayout ret;
    ret.internalLayout.begin();
    for (size_t i = 0; i < descriptorsLength; i++)
        ret.internalLayout.add(descriptors[i].attribute, descriptors[i].count, descriptors[i].type, descriptors[i].normalize);
    ret.internalLayout.end();
    return ret;
}

StaticMeshHandle StaticMeshHandle::create(const void* vertexData, uint32_t vertexDataSize, VertexLayout vl, const uint16_t* indexData, uint32_t indexDataSize)
{
    StaticMeshHandle ret;
    char* vertData = new char[vertexDataSize];
    memcpy(vertData, vertexData, vertexDataSize);
    char* indData = new char[indexDataSize];
    memcpy(indData, indexData, indexDataSize);
    ret.internalVertexBuffer = bgfx::createVertexBuffer(bgfx::makeRef(vertData, vertexDataSize, [](void* data, void*) { delete[] static_cast<char*>(data); }), vl.internalLayout);
    ret.internalIndexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(indData, indexDataSize, [](void* data, void*) { delete[] static_cast<char*>(data); }));
    return ret;
}
void StaticMeshHandle::destroy()
{
    bgfx::destroy(this->internalVertexBuffer);
    bgfx::destroy(this->internalIndexBuffer);
}

DynamicMeshHandle DynamicMeshHandle::create(const void* vertexData, uint32_t vertexDataSize, VertexLayout vl, const uint16_t* indexData, uint32_t indexDataSize)
{
    DynamicMeshHandle ret;
    char* vertData = new char[vertexDataSize];
    memcpy(vertData, vertexData, vertexDataSize);
    char* indData = new char[indexDataSize];
    memcpy(indData, indexData, indexDataSize);
    ret.internalVertexBuffer = bgfx::createDynamicVertexBuffer(bgfx::makeRef(vertData, vertexDataSize, [](void* data, void*) { delete[] static_cast<char*>(data); }), vl.internalLayout, BGFX_BUFFER_ALLOW_RESIZE);
    ret.internalIndexBuffer = bgfx::createDynamicIndexBuffer(bgfx::makeRef(indData, indexDataSize, [](void* data, void*) { delete[] static_cast<char*>(data); }), BGFX_BUFFER_ALLOW_RESIZE);
    return ret;
}
void DynamicMeshHandle::update(const void* vertexData, uint32_t vertexDataSize, const uint16_t* indexData, uint32_t indexDataSize, uint32_t fromVertex, uint32_t fromIndex)
{
    char* vertData = new char[vertexDataSize];
    memcpy(vertData, vertexData, vertexDataSize);
    char* indData = new char[indexDataSize];
    memcpy(indData, indexData, indexDataSize);
    bgfx::update(this->internalVertexBuffer, fromVertex, bgfx::makeRef(vertData, vertexDataSize, [](void* data, void*) { delete[] static_cast<char*>(data); }));
    bgfx::update(this->internalIndexBuffer, fromIndex, bgfx::makeRef(indData, indexDataSize, [](void* data, void*) { delete[] static_cast<char*>(data); }));
}
void DynamicMeshHandle::destroy()
{
    bgfx::destroy(this->internalVertexBuffer);
    bgfx::destroy(this->internalIndexBuffer);
}