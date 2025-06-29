#include <Components/EntityAttributes.h>
#include <Firework.Core.hpp>
#include <Firework/Entry.h>
#include <GL/Geometry.h>
#include <GL/Renderer.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;
using namespace Firework::PackageSystem;

struct FilledPathRenderer
{
    StaticMeshHandle fill;
    GeometryProgramHandle program;

    inline operator bool()
    {
        return this->program && this->fill;
    }

    inline void draw(const RenderTransform& transform)
    {
        Renderer::setDrawStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0xff) | BGFX_STENCIL_OP_FAIL_S_INVERT | BGFX_STENCIL_OP_FAIL_Z_KEEP |
                                 BGFX_STENCIL_OP_PASS_Z_INVERT);
        Renderer::submitDraw(128u, this->fill, this->program);
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
    InternalEngineEvent::OnRenderOffloadForComponent += [](std::type_index type, std::shared_ptr<void> component)
    {
        if (type == std::type_index(typeid(TestComponent)))
        {
            std::cout << "wow!\n";
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
    };
    EngineEvent::OnWindowResize += []([[maybe_unused]] sysm::vector2i32 from)
    {
    };

    return 0;
}
