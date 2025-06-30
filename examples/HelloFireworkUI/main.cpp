#include <Components/EntityAttributes.h>
#include <Core/CoreEngine.h>
#include <Firework.Core.hpp>
#include <Firework/Entry.h>
#include <Friends/FilledPathRenderer.h>
#include <GL/Geometry.h>
#include <GL/Renderer.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;
using namespace Firework::PackageSystem;

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
    };
    EngineEvent::OnWindowResize += []([[maybe_unused]] sysm::vector2i32 from)
    {
    };

    return 0;
}
