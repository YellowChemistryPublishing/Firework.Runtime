#include "Renderer.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <Mathematics.h>

#include <DebugCube.vfAll.h>

using namespace Firework::Mathematics;
using namespace Firework::GL;

#if _DEBUG
static struct Vertex
{
    float x, y, z;
}
verts[]
{
    { 0.5f,  0.5f,  0.5f },
    { 0.5f,  0.5f,  -0.5f },
    { 0.5f,  -0.5f, 0.5f },
    { 0.5f,  -0.5f, -0.5f },
    { -0.5f, 0.5f,  0.5f },
    { -0.5f, 0.5f,  -0.5f },
    { -0.5f, -0.5f, 0.5f },
    { -0.5f, -0.5f, -0.5f }
};
static uint16_t inds[]
{
    7, 1, 5,
    7, 3, 1,
    3, 0, 1,
    3, 2, 0,
    6, 7, 5,
    6, 5, 4,
    2, 4, 0,
    2, 6, 4,
    5, 0, 4,
    5, 1, 0,
    6, 2, 3,
    6, 3, 7
};
static VertexLayout layout = VertexLayout::create({ { bgfx::Attrib::Position, bgfx::AttribType::Float, 3 } });
static StaticMeshHandle cubeMesh;
static GeometryProgramHandle cubeProgram;
#endif

bool Renderer::initialize(void* ndt, void* nwh, uint32_t width, uint32_t height, RendererBackend backend, uint32_t initFlags)
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
    init.resolution.width = width;
    init.resolution.height = height;
    init.resolution.reset = initFlags;
    init.platformData = pd;

    if (!bgfx::init(init))
        return false;

    #if _DEBUG
    cubeMesh = StaticMeshHandle::create(verts, sizeof(verts), layout, inds, sizeof(inds));
    switch (bgfx::getRendererType())
    {
    #if _WIN32
    case bgfx::RendererType::Direct3D11:
        cubeProgram = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(DebugCube, d3d11));
        break;
    case bgfx::RendererType::Direct3D12:
        cubeProgram = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(DebugCube, d3d12));
        break;
    #endif
    case bgfx::RendererType::OpenGL:
        cubeProgram = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(DebugCube, opengl));
        break;
    case bgfx::RendererType::Vulkan:
        cubeProgram = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(DebugCube, vulkan));
        break;
    default:
        // TODO: Implement.
        throw "unimplemented";
    }
    #endif

    bx::Vec3 eye { 0.0f, 0.0f, -1.0f }, at { 0.0f, 0.0f, 0.0f };
    float view[16];
    float proj[16];
    bx::mtxLookAt(view, eye, at);
    bx::mtxOrtho(proj, -float(width) / 2.0f, float(width) / 2.0f, -float(height) / 2.0f, float(height) / 2.0f, -1.0f, 2048.0f, 0, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view, proj);
    bgfx::setViewRect(0, 0, 0, width, height);

    return true;
}
void Renderer::shutdown()
{
    #if _DEBUG
    cubeMesh.destroy();
    cubeProgram.destroy();
    #endif

    bgfx::shutdown();
}

void Renderer::showDebugInformation()
{
    bgfx::setDebug(BGFX_DEBUG_PROFILER | BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT);
}
void Renderer::hideDebugInformation()
{
    bgfx::setDebug(BGFX_DEBUG_NONE);
}

RendererBackend Renderer::rendererBackend()
{
    return (RendererBackend)bgfx::getRendererType();
}
std::vector<RendererBackend> Renderer::platformBackends()
{
    RendererBackend backends[(size_t)RendererBackend::Default];
    uint8_t end = bgfx::getSupportedRenderers((uint8_t)RendererBackend::Default, reinterpret_cast<bgfx::RendererType::Enum*>(backends));
    return std::vector<RendererBackend>(backends, backends + end);
}

void Renderer::setViewOrthographic(bgfx::ViewId id, float width, float height, Vector3 position, Quaternion rotation, float near, float far)
{
    // fixme
    float viewMtx[16];
    float proj[16]; 
    bx::Vec3 eye { 0, 0, 0 }, at { 0, 0, 1 };
    bx::mtxLookAt(viewMtx, eye, at);
    bx::mtxOrtho(proj, -width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, near, far, 0, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(id, viewMtx, proj);
}
void Renderer::setViewPerspective(bgfx::ViewId id, float width, float height, float yFieldOfView, Vector3 position, Quaternion rotation, float near, float far)
{
    float proj[16] { 0 };
    bx::mtxProj(proj, yFieldOfView, width / height, near, far, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(id, (Matrix4x4::rotate(rotation) * Matrix4x4::translate(position)).data, proj);
}
void Renderer::setViewClear(bgfx::ViewId id, uint32_t rgbaColor, uint16_t flags, float depth, uint8_t stencil)
{
    bgfx::setViewClear(id, flags, rgbaColor, depth, stencil);
}
void Renderer::setViewArea(bgfx::ViewId id, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    bgfx::setViewRect(id, x, y, width, height);
}
void Renderer::resetBackbuffer(uint32_t width, uint32_t height, uint32_t flags, bgfx::TextureFormat::Enum format)
{
    bgfx::reset(width, height, flags, format);
}

void Renderer::setDrawTransform(const RenderTransform& transform)
{
    bgfx::setTransform(transform.tf.data);
}
void Renderer::setDrawUniform(UniformHandle uniform, const void* data)
{
    bgfx::setUniform(uniform.internalHandle, data);
}
void Renderer::setDrawArrayUniform(UniformHandle uniform, const void* data, uint16_t count)
{
    bgfx::setUniform(uniform.internalHandle, data, count);
}
void Renderer::setDrawTexture(uint8_t stage, Texture2DHandle texture, TextureSamplerHandle sampler, uint64_t flags)
{
    bgfx::setTexture(stage, sampler.internalHandle, texture.internalHandle, flags);
}
void Renderer::submitDraw(bgfx::ViewId id, StaticMeshHandle mesh, GeometryProgramHandle program, uint64_t state, uint32_t blendFactor)
{
    bgfx::setVertexBuffer(0, mesh.internalVertexBuffer);
    bgfx::setIndexBuffer(mesh.internalIndexBuffer);
    bgfx::setState(state, blendFactor);
    bgfx::submit(id, program.internalHandle);
}
void Renderer::submitDraw(bgfx::ViewId id, DynamicMeshHandle mesh, GeometryProgramHandle program, uint64_t state, uint32_t blendFactor)
{
    bgfx::setVertexBuffer(0, mesh.internalVertexBuffer);
    bgfx::setIndexBuffer(mesh.internalIndexBuffer);
    bgfx::setState(state, blendFactor);
    bgfx::submit(id, program.internalHandle);
}

#if _DEBUG
void Renderer::debugDrawCube(Vector3 position, float sideLength)
{
    RenderTransform transform;
    transform.scale(Vector3(sideLength));
    transform.translate(position);
    Renderer::setDrawTransform(transform);
    Renderer::submitDraw(0, cubeMesh, cubeProgram);
}
#endif

void Renderer::drawFrame()
{
    bgfx::touch(0);
    bgfx::frame();
}

Quaternion Renderer::fromEuler(Vector3 vec)
{
    bx::Quaternion ret = bx::fromEuler({ vec.x, vec.y, vec.z });
    return { ret.x, ret.y, ret.z, ret.w };
}
