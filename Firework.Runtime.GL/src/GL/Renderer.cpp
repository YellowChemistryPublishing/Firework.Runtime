#include "Renderer.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <glm/gtc/type_ptr.hpp>
#include <module/sys>

#include <GL/Geometry.h>
#include <GL/Shader.h>
#include <GL/Texture.h>

#include <DebugCube.vfAll.h>

using namespace Firework::GL;

std::vector<std::pair<void (*)(bgfx::ViewId, void*), void*>> Renderer::drawPassIntercepts;

#if _DEBUG
static struct Vertex
{
    float x, y, z;
} verts[] { { 0.5f, 0.5f, 0.5f },  { 0.5f, 0.5f, -0.5f },  { 0.5f, -0.5f, 0.5f },  { 0.5f, -0.5f, -0.5f },
            { -0.5f, 0.5f, 0.5f }, { -0.5f, 0.5f, -0.5f }, { -0.5f, -0.5f, 0.5f }, { -0.5f, -0.5f, -0.5f } };
static uint16_t inds[] { 7, 1, 5, 7, 3, 1, 3, 0, 1, 3, 2, 0, 6, 7, 5, 6, 5, 4, 2, 4, 0, 2, 6, 4, 5, 0, 4, 5, 1, 0, 6, 2, 3, 6, 3, 7 };

static VertexLayout layout = VertexLayout(std::array { VertexDescriptor { VertexAttributeName::Position, VertexAttributeType::Float, 3 } });
static StaticMesh cubeMesh = nullptr;
static GeometryProgram cubeProgram = nullptr;
#endif

bool Renderer::initialize(void* ndt, void* nwh, u32 width, u32 height, RendererBackend backend, u32 initFlags)
{
    bgfx::PlatformData pd;
    pd.ndt = ndt;
    pd.nwh = nwh;
    pd.context = nullptr;
    pd.backBuffer = nullptr;
    pd.backBufferDS = nullptr;

    bgfx::Init init;
    init.type = (bgfx::RendererType::Enum)backend;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.resolution.width = +width;
    init.resolution.height = +height;
    init.resolution.reset = +initFlags;
    init.platformData = pd;

    if (!bgfx::init(init))
        return false;

#if _DEBUG
    cubeMesh = StaticMesh(std::span(_asr(byte*, &verts), sizeof(verts)), layout, std::span(inds));
    createShaderFromPrecompiled(cubeProgram, DebugCube);
#endif

    bx::Vec3 eye { 0.0f, 0.0f, -1.0f }, at { 0.0f, 0.0f, 0.0f };
    float view[16];
    float proj[16];
    bx::mtxLookAt(view, eye, at);
    bx::mtxOrtho(proj, -float(width) / 2.0f, float(width) / 2.0f, -float(height) / 2.0f, float(height) / 2.0f, -1.0f, 2048.0f, 0, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view, proj);
    bgfx::setViewRect(0, 0, 0, +u16(width), +u16(height));

    return true;
}
void Renderer::shutdown()
{
#if _DEBUG
    cubeMesh = nullptr;
    cubeProgram = nullptr;
#endif

    bgfx::shutdown();
}

static u32 debugFlags = BGFX_DEBUG_NONE;
void Renderer::showDebugInformation(bool visible)
{
    if (visible)
    {
        debugFlags |= u32(BGFX_DEBUG_PROFILER | BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT);
        bgfx::setDebug(+debugFlags);
    }
    else
    {
        debugFlags &= ~u32(BGFX_DEBUG_PROFILER | BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT);
        bgfx::setDebug(+debugFlags);
    }
}
void Renderer::showDebugWireframes(bool visible)
{
    if (visible)
    {
        debugFlags |= u32(BGFX_DEBUG_WIREFRAME);
        bgfx::setDebug(+debugFlags);
    }
    else
    {
        debugFlags &= ~u32(BGFX_DEBUG_WIREFRAME);
        bgfx::setDebug(+debugFlags);
    }
}

RendererBackend Renderer::rendererBackend()
{
    return _as(RendererBackend, bgfx::getRendererType());
}
std::vector<RendererBackend> Renderer::platformBackends()
{
    RendererBackend backends[size_t(RendererBackend::Default)];
    u8 end = bgfx::getSupportedRenderers(uint8_t(RendererBackend::Default), _asr(bgfx::RendererType::Enum*, backends));
    return std::vector<RendererBackend>(backends, backends + +end);
}

void Renderer::setViewOrthographic(bgfx::ViewId id, float width, float height, glm::vec3 position, glm::quat rotation, float near, float far)
{
    glm::mat4 view = glm::translate(glm::mat4(1.0f), position);
    view *= glm::mat4_cast(rotation);

    float proj[16];
    bx::mtxOrtho(proj, -width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, near, far, 0, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(id, glm::value_ptr(view), proj);
}
void Renderer::setViewPerspective(bgfx::ViewId id, float width, float height, float yFieldOfView, glm::vec3 position, glm::quat rotation, float near, float far)
{
    float proj[16] { 0 };
    bx::mtxProj(proj, yFieldOfView, width / height, near, far, bgfx::getCaps()->homogeneousDepth);

    glm::mat4 viewTransform = glm::translate(glm::mat4_cast(rotation), position);
    bgfx::setViewTransform(id, glm::value_ptr(viewTransform), proj);
}

void Renderer::setViewClear(bgfx::ViewId id, u32 rgbaColor, u16 flags, float depth, byte stencil)
{
    bgfx::setViewClear(id, +flags, +rgbaColor, depth, stencil);
}
void Renderer::setViewArea(bgfx::ViewId id, u16 x, u16 y, u16 width, u16 height)
{
    bgfx::setViewRect(id, +x, +y, +width, +height);
}
void Renderer::setViewDrawOrder(bgfx::ViewId id, bgfx::ViewMode::Enum order)
{
    bgfx::setViewMode(id, order);
}
void Renderer::resetBackbuffer(u32 width, u32 height, u32 flags, bgfx::TextureFormat::Enum format)
{
    bgfx::reset(+width, +height, +flags, format);
}

void Renderer::setDrawTransform(const glm::mat4& transform)
{
    bgfx::setTransform(glm::value_ptr(transform));
}
bool Renderer::setDrawUniform(const Uniform& uniform, const void* data)
{
    _fence_value_return(false, !uniform);

    bgfx::setUniform(uniform.internalHandle, data);
    return true;
}
bool Renderer::setDrawArrayUniform(const Uniform& uniform, const void* data, u16 count)
{
    _fence_value_return(false, !uniform);

    bgfx::setUniform(uniform.internalHandle, data, +count);
    return true;
}
bool Renderer::setDrawTexture(u8 stage, const Texture2D& texture, const TextureSampler& sampler, u32 flags)
{
    _fence_value_return(false, !texture || !sampler);

    bgfx::setTexture(+stage, sampler.internalHandle, texture.internalHandle, +flags);
    return true;
}
void Renderer::setDrawStencil(u32 func, u32 back)
{
    bgfx::setStencil(+func, +back);
}

void Renderer::addDrawPassIntercept(void (*intercept)(bgfx::ViewId, void*), void* data)
{
    Renderer::drawPassIntercepts.push_back(std::make_pair(intercept, data));
}
void Renderer::removeDrawPassIntercept(void (*intercept)(bgfx::ViewId, void*))
{
    for (auto it = Renderer::drawPassIntercepts.begin(); it != Renderer::drawPassIntercepts.end(); ++it)
    {
        if (it->first == intercept)
        {
            Renderer::drawPassIntercepts.erase(it);
            break;
        }
    }
}

bool Renderer::submitDraw(bgfx::ViewId id, const StaticMesh& mesh, const GeometryProgram& program, u64 state, u32 blendFactor)
{
    _fence_value_return(false, !mesh || !program);

    bgfx::setVertexBuffer(0, mesh.internalVertexBuffer);
    bgfx::setIndexBuffer(mesh.internalIndexBuffer);
    bgfx::setState(+state, +blendFactor);
    for (auto& [intercept, data] : Renderer::drawPassIntercepts) intercept(id, data);
    bgfx::submit(id, program.internalHandle);
    return true;
}
bool Renderer::submitDraw(bgfx::ViewId id, const DynamicMesh& mesh, const GeometryProgram& program, u64 state, u32 blendFactor)
{
    _fence_value_return(false, !mesh || !program);

    bgfx::setVertexBuffer(0, mesh.internalVertexBuffer);
    bgfx::setIndexBuffer(mesh.internalIndexBuffer);
    bgfx::setState(+state, +blendFactor);
    for (auto& [intercept, data] : Renderer::drawPassIntercepts) intercept(id, data);
    bgfx::submit(id, program.internalHandle);
    return true;
}

#if _DEBUG
void Renderer::debugDrawCube(glm::vec3 position, float sideLength)
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
    transform = glm::scale(transform, glm::vec3(sideLength));
    Renderer::setDrawTransform(transform);
    Renderer::submitDraw(0, cubeMesh, cubeProgram);
}
#endif

void Renderer::drawFrame()
{
    bgfx::touch(0);
    bgfx::frame();
}
