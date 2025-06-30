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

struct RendererUtil
{
    inline static float unitSquareVerts[] { -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
    inline static uint16_t unitSquareInds[] { 2, 1, 0, 3, 2, 0 };
    inline static StaticMeshHandle unitSquare = nullptr;
};

struct FilledPathPoint
{

};

class FilledPathRenderer
{
    inline static float testVerts[] { 85, 630, 1,   85, 0,   1,   436, 0,   1,   461, 72,  1,   461, 80,  1,   180, 80,  1,   180, 285, 1,   401, 285,
                                      1,  423, 353, 1,  423, 361, 1,   180, 361, 1,   180, 551, 1,   433, 551, 1,   456, 622, 1,   456, 630, 1 };
    StaticMeshHandle fill;

    inline static GeometryProgramHandle program;
public:
    inline operator bool()
    {
        return this->program && this->fill;
    }

    inline bool createPolygon()
    {

    }

    inline void draw(const RenderTransform& transform)
    {
        Renderer::setDrawStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0xff) | BGFX_STENCIL_OP_FAIL_S_INVERT | BGFX_STENCIL_OP_FAIL_Z_KEEP |
                                 BGFX_STENCIL_OP_PASS_Z_INVERT);
        Renderer::submitDraw(1u, this->fill, FilledPathRenderer::program);
    }
};

struct TestComponent
{
    struct RenderData
    {
        FilledPathRenderer path;
    };
    std::shared_ptr<RenderData> renderData;
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    InternalEngineEvent::OnRenderOffloadForComponent += [](std::type_index type, Entity& entity, std::shared_ptr<void> component)
    {
        if (type == std::type_index(typeid(TestComponent)))
        {
            CoreEngine::queueRenderJobForFrame([]
            {
                RenderTransform t;
                t.scale(0.1f);

                float col[4] { 1.0f, 1.0f, 1.0f, 1.0f };
                FilledPathRenderer::program.setUniform("u_color", &col);
                Renderer::setDrawTransform(t);
                Renderer::setDrawStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0) | BGFX_STENCIL_OP_FAIL_S_INVERT |
                                         BGFX_STENCIL_OP_FAIL_Z_INVERT | BGFX_STENCIL_OP_PASS_Z_INVERT);
                Renderer::submitDraw(1, FilledPathRenderer::fill, FilledPathRenderer::program, BGFX_STATE_DEPTH_TEST_LESS);

                RenderTransform t2;
                t2.scale(500.0f);

                FilledPathRenderer::program.setUniform("u_color", &col);
                Renderer::setDrawTransform(t2);
                Renderer::setDrawStencil(BGFX_STENCIL_TEST_LESS | BGFX_STENCIL_FUNC_REF(0b10000000) | BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
                                         BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP);
                Renderer::submitDraw(1, RendererUtil::unitSquare, FilledPathRenderer::program);
            }, false);
        }
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

        CoreEngine::queueRenderJobForFrame([]
        {
            RendererUtil::unitSquare =
                StaticMeshHandle::create(RendererUtil::unitSquareVerts, sizeof(RendererUtil::unitSquareVerts),
                                         VertexLayout::create({ VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 } }),
                                         RendererUtil::unitSquareInds, sizeof(RendererUtil::unitSquareInds));

            std::vector<uint16_t> testInds;
            for (ssz i = 1; i < ssz(sizeof(FilledPathRenderer::testVerts) / sizeof(float) / 3) - 1_z; i++)
            {
                testInds.push_back(+0_u16);
                testInds.push_back(+u16(i + 1_z));
                testInds.push_back(+u16(i));
            }
            FilledPathRenderer::fill =
                StaticMeshHandle::create(FilledPathRenderer::testVerts, sizeof(FilledPathRenderer::testVerts),
                                         VertexLayout::create({ VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 } }),
                                         testInds.data(), testInds.size() * sizeof(uint16_t));

            switch (Renderer::rendererBackend())
            {
#if _WIN32
            case RendererBackend::Direct3D11:
                FilledPathRenderer::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Panel, d3d11),
                                                                            { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
                break;
            case RendererBackend::Direct3D12:
                FilledPathRenderer::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Panel, d3d12),
                                                                            { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
                break;
#endif
            case RendererBackend::OpenGL:
                FilledPathRenderer::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Panel, opengl),
                                                                            { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
                break;
            case RendererBackend::Vulkan:
                FilledPathRenderer::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Panel, vulkan),
                                                                            { ShaderUniform { .name = "u_color", .type = UniformType::Vec4 } });
                break;
            default:
                // TODO: Implement.
                throw "unimplemented";
            }
        });
    };
    EngineEvent::OnWindowResize += []([[maybe_unused]] sysm::vector2i32 from)
    {
    };

    return 0;
}
