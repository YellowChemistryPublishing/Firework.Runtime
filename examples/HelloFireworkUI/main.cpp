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
        FilledPathRenderer path1;
        FilledPathRenderer path2;
        FilledPathRenderer path3;

        inline RenderData()
        {
            CoreEngine::queueRenderJobForFrame([renderData = this]
            {
                FilledPathPoint testVerts1[] { { 945, 1438 }, { 644, 1440 }, { 454, 1290 }, { 268, 1142 }, { 268, 911 },  { 268, 646 },   { 415, 531 },   { 546, 427 },
                                               { 818, 429 },  { 1039, 430 }, { 1200, 630 }, { 1343, 807 }, { 1343, 993 }, { 1343, 1191 }, { 1250, 1307 }, { 1146, 1437 } };
                renderData->path1 = FilledPathRenderer(std::span(testVerts1));
                FilledPathPoint testVerts2[] { { 903, 1620 }, { 1192, 1620 }, { 1361, 1434 }, { 1522, 1257 }, { 1522, 973 }, { 1522, 725 }, { 1320, 498 },
                                               { 1102, 254 }, { 818, 251 },   { 90, 243 },    { 90, 911 },    { 90, 1183 },  { 586, 1620 } };
                renderData->path2 = FilledPathRenderer(std::span(testVerts2));
                FilledPathPoint testVerts3[] { { 893, 1139 },  { 779, 1139 },  { 689, 1084 },  { 596, 1026 }, { 596, 944 },  { 596, 887 },  { 661, 848 },
                                               { 718, 814 },   { 779, 814 },   { 853, 814 },   { 1008, 937 }, { 1029, 937 }, { 1066, 937 }, { 1120, 880 },
                                               { 1120, 844 },  { 1120, 774 },  { 893, 638 },   { 779, 636 },  { 643, 633 },  { 534, 719 },  { 418, 810 },
                                               { 418, 944 },   { 418, 1101 },  { 577, 1214 },  { 723, 1318 }, { 886, 1318 }, { 942, 1318 }, { 983, 1290 },
                                               { 1030, 1258 }, { 1030, 1205 }, { 1030, 1128 }, { 952, 1128 }, { 941, 1128 }, { 903, 1139 } };
                renderData->path3 = FilledPathRenderer(std::span(testVerts3));
            });
        }
    };
    std::shared_ptr<RenderData> renderData;

    inline TestComponent() : renderData(std::make_shared<RenderData>())
    { }
    inline ~TestComponent()
    {
        // Must destroy `this->renderData` in the render thread.
        CoreEngine::queueRenderJobForFrame([renderData = this->renderData] { });
    }

    void renderOffload(sz renderIndex = 0)
    {
        CoreEngine::queueRenderJobForFrame([renderIndex, renderData = this->renderData]
        {
            RenderTransform t1;
            t1.scale(0.25f);
            if (!renderData->path1.submitDrawStencil(renderIndex, t1))
                return;
            if (!renderData->path2.submitDrawStencil(renderIndex, t1))
                return;
            if (!renderData->path3.submitDrawStencil(renderIndex, t1))
                return;

            RenderTransform t2;
            t2.scale(500.0f);
            (void)FilledPathRenderer::submitDraw(renderIndex, t2, ~u8(0));
        }, false);
    }
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    InternalEngineEvent::OnRenderOffloadForComponent += [](std::type_index type, Entity&, std::shared_ptr<void> component, sz renderIndex)
    {
        if (type == std::type_index(typeid(TestComponent)))
            std::static_pointer_cast<TestComponent>(component)->renderOffload(renderIndex);
    };

    EngineEvent::OnInitialize += []
    {
        auto e = Entity::alloc();
        e->addComponent<EntityAttributes>()->name = "beans";
        Entity::alloc()->addComponent<EntityAttributes>()->name = "beans2";
        auto e3 = Entity::alloc();
        e3->addComponent<EntityAttributes>()->name = "beans3";
        e3->parent = e;
        e3->addComponent<TestComponent>();

        Debug::printHierarchy();
    };

    return 0;
}
