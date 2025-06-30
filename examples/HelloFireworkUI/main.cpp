#include <Components/EntityAttributes.h>
#include <Core/CoreEngine.h>
#include <Firework.Core.hpp>
#include <Firework/Entry.h>
#include <GL/Geometry.h>
#include <GL/Renderer.h>

#include <Panel.vfAll.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;
using namespace Firework::PackageSystem;

_pack(1) struct FilledPathPoint
{
    float x, y, z = 1.0f; // Leave as 1.0f unless you're really confident in what you're doing.
};

class FilledPathRenderer
{
    inline static GeometryProgramHandle program;
    inline static StaticMeshHandle unitSquare;

    StaticMeshHandle fill = nullptr;
public:
    static bool renderInitialize()
    {
        InternalEngineEvent::OnRenderShutdown += []
        {
            if (FilledPathRenderer::program) [[likely]]
                FilledPathRenderer::program.destroy();
            if (FilledPathRenderer::unitSquare) [[likely]]
                FilledPathRenderer::unitSquare.destroy();
        };

        switch (Renderer::rendererBackend())
        {
#if _WIN32
        case RendererBackend::Direct3D11:
            FilledPathRenderer::program =
                GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Panel, d3d11), { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
            break;
        case RendererBackend::Direct3D12:
            FilledPathRenderer::program =
                GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Panel, d3d12), { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
            break;
#endif
        case RendererBackend::OpenGL:
            FilledPathRenderer::program =
                GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Panel, opengl), { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
            break;
        case RendererBackend::Vulkan:
            FilledPathRenderer::program =
                GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Panel, vulkan), { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
            break;
        default:;
        }

        float unitSquareVerts[] { -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
        uint16_t unitSquareInds[] { 2, 1, 0, 3, 2, 0 };
        FilledPathRenderer::unitSquare =
            StaticMeshHandle::create(unitSquareVerts, sizeof(unitSquareVerts),
                                     VertexLayout::create({ VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 } }),
                                     unitSquareInds, sizeof(unitSquareInds));

        return FilledPathRenderer::program && FilledPathRenderer::unitSquare;
    }

    inline FilledPathRenderer(std::nullptr_t = nullptr)
    { }
    template <typename Container>
    requires sys::IEnumerable<Container, FilledPathPoint> && sys::ISizeable<Container>
    inline FilledPathRenderer(Container&& closedPath)
    {
        std::vector<FilledPathPoint> verts;
        if constexpr (requires { sz(std::size(closedPath)); })
            verts.reserve(+sz(std::size(closedPath)));
        verts.insert(verts.begin(), std::begin(closedPath), std::end(closedPath));

        std::vector<uint16_t> inds;
        if constexpr (requires { sz(std::size(closedPath)); })
            inds.reserve(+((sz(std::size(closedPath)) - 2_uz) * 3_uz));

        for (u16 i = 1; i < u16(std::size(closedPath)) - 1_u16; i++)
        {
            inds.push_back(0);
            inds.push_back(+(i + 1_u16));
            inds.push_back(+i);
        }

        this->fill = StaticMeshHandle::create(std::data(verts), std::size(verts) * sizeof(decltype(verts)::value_type),
                                              VertexLayout::create({ VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 } }),
                                              std::data(inds), std::size(inds) * sizeof(decltype(inds)::value_type));
    }
    inline FilledPathRenderer(const FilledPathRenderer&) = delete;
    inline FilledPathRenderer(FilledPathRenderer&& other)
    {
        swap(*this, other);
    }
    inline ~FilledPathRenderer()
    {
        if (this->fill) [[likely]]
            this->fill.destroy();
    }

    inline operator bool()
    {
        return this->fill;
    }

    inline FilledPathRenderer& operator=(const FilledPathRenderer&) = delete;
    inline FilledPathRenderer& operator=(FilledPathRenderer&& other)
    {
        swap(*this, other);
        return *this;
    }

    inline void submitDrawStencil(sz renderIndex, RenderTransform shape, bool forceHole = false)
    {
        float col[4] { 1.0f, 1.0f, 1.0f, 1.0f };
        FilledPathRenderer::program.setUniform("u_color", &col);

        shape.translate(sysm::vector3::forward * float(+renderIndex));
        Renderer::setDrawTransform(shape);

        Renderer::setDrawStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0) |
                                 (forceHole ? BGFX_STENCIL_OP_FAIL_S_REPLACE | BGFX_STENCIL_OP_FAIL_Z_REPLACE | BGFX_STENCIL_OP_PASS_Z_REPLACE
                                            : BGFX_STENCIL_OP_FAIL_S_INVERT | BGFX_STENCIL_OP_FAIL_Z_INVERT | BGFX_STENCIL_OP_PASS_Z_INVERT));
        Renderer::submitDraw(1, this->fill, FilledPathRenderer::program, BGFX_STATE_DEPTH_TEST_LESS);
    }
    inline static void submitDraw(sz renderIndex, RenderTransform clip, u8 whenStencil)
    {
        float col[4] { 1.0f, 1.0f, 1.0f, 1.0f };
        FilledPathRenderer::program.setUniform("u_color", &col);

        clip.translate(sysm::vector3::forward * float(+renderIndex));
        Renderer::setDrawTransform(clip);

        Renderer::setDrawStencil(BGFX_STENCIL_TEST_EQUAL | BGFX_STENCIL_FUNC_REF(+whenStencil) | BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
                                 BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP);
        Renderer::submitDraw(1, FilledPathRenderer::unitSquare, FilledPathRenderer::program);
    }

    friend inline void swap(FilledPathRenderer& a, FilledPathRenderer& b)
    {
        using std::swap;

        swap(a.fill, b.fill);
    }
};

struct TestComponent
{
    struct RenderData : public std::enable_shared_from_this<RenderData>
    {
        FilledPathRenderer path;

        inline RenderData()
        {
            CoreEngine::queueRenderJobForFrame([renderData = this]
            {
                FilledPathPoint testVerts[] {
                    { 414, 638 }, { 180, 338 }, { 180, 623 }, { 96, 637 }, { 85, 637 }, { 85, 4 },    { 170, -4 },  { 180, -4 },
                    { 180, 219 }, { 250, 306 }, { 425, 3 },   { 526, -4 }, { 535, -4 }, { 311, 374 }, { 512, 623 }, { 424, 638 },
                };
                renderData->path = FilledPathRenderer(std::span(testVerts));
            });
        }
    };
    std::shared_ptr<RenderData> renderData;

    inline TestComponent() : renderData(std::make_shared<RenderData>())
    { }

    void renderOffload(sz renderIndex = 0)
    {
        CoreEngine::queueRenderJobForFrame([renderIndex, renderData = this->renderData]
        {
            RenderTransform t1;
            t1.scale(0.25f);
            renderData->path.submitDrawStencil(renderIndex, t1);

            RenderTransform t2;
            t2.scale(500.0f);
            FilledPathRenderer::submitDraw(renderIndex, t2, ~u8(0));
        }, false);
    }
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    InternalEngineEvent::OnRenderOffloadForComponent += [](std::type_index type, Entity& entity, std::shared_ptr<void> component, sz renderIndex)
    {
        if (type == std::type_index(typeid(TestComponent)))
            std::static_pointer_cast<TestComponent>(component)->renderOffload(renderIndex);
    };

    EngineEvent::OnInitialize += []
    {
        auto e = (new Entity())->shared_from_this();
        e->addComponent<EntityAttributes>()->name = "beans";
        (new Entity())->addComponent<EntityAttributes>()->name = "beans2";
        auto e3 = (new Entity())->shared_from_this();
        e3->addComponent<EntityAttributes>()->name = "beans3";
        e3->parent = e;
        e3->addComponent<TestComponent>();

        Debug::printHierarchy();

        CoreEngine::queueRenderJobForFrame([] { FilledPathRenderer::renderInitialize(); });
    };
    EngineEvent::OnWindowResize += []([[maybe_unused]] sysm::vector2i32 from)
    {
    };

    return 0;
}
