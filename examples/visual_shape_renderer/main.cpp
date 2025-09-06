#include "../common.h"

#include <array>

#include <Firework.Components.Core2D>
#include <Firework.Runtime.CoreLib>

namespace fs = std::filesystem;

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::PackageSystem;

struct ShapeRendererTestComponent
{
    std::shared_ptr<ShapeRenderer> rend;
};

int main(int, char*[])
{
    hookExampleControls();

    EngineEvent::OnInitialize += []
    {
        auto entity = Entity::alloc();
        entity->addComponent<EntityAttributes>()->name = "Test Entity";

        auto sr = entity->addComponent<ShapeRendererTestComponent>();
        sr->rend = std::make_shared<ShapeRenderer>(
            std::array { ShapePoint { .x = -100.25f, .y = 0.25f, .xCtrl = -1.0f, .yCtrl = 1.0f }, ShapePoint { .x = 0.25f, .y = 20.25f, .xCtrl = 0.0f, .yCtrl = -1.0f },
                         ShapePoint { .x = 100.25f, .y = 0.25f, .xCtrl = 1.0f, .yCtrl = 1.0f }, ShapePoint { .x = -50.25f, .y = -100.25f, .xCtrl = -1.0f, .yCtrl = 1.0f },
                         ShapePoint { .x = 0.25f, .y = 0.25f, .xCtrl = 0.0f, .yCtrl = -1.0f }, ShapePoint { .x = -50.25f, .y = 100.25f, .xCtrl = 1.0f, .yCtrl = 1.0f } },
            std::array<uint16_t, 6> { 0, 1, 2, 3, 4, 5 }, 0_u32);

        auto rectTransform = entity->getOrAddComponent<RectTransform>();
        rectTransform->rect =
            RectFloat(float(Window::pixelHeight()) / 2.0f, float(Window::pixelWidth()) / 2.0f, -float(Window::pixelHeight()) / 2.0f, -float(Window::pixelWidth()) / 2.0f);

        Debug::printHierarchy();
    };
    InternalEngineEvent::OnLateRenderOffloadForComponent += [](std::type_index type, Entity& entity, std::shared_ptr<void> component, ssz ri)
    {
        if (type != typeid(ShapeRendererTestComponent))
            return;

        auto sr = std::static_pointer_cast<ShapeRendererTestComponent>(component);
        if (!sr->rend)
            return;

        CoreEngine::queueRenderJobForFrame([tf = entity.getOrAddComponent<RectTransform>()->matrix(), rend = sr->rend, ri = float(+ri)]
        {
            _push_nowarn_c_cast();

            (void)rend->submitDrawCover(ri, tf, 0_u8, Color(0, 0, 0, 0),
                                        BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_ZERO), 0);
            (void)rend->submitDrawStencil(ri, glm::mat4(1.0f), FillRule::EvenOdd);
            (void)rend->submitDrawCover(ri, tf, 0_u8, Color(255, 255, 255, 255),
                                        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS |
                                            BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_DST_ALPHA, BGFX_STATE_BLEND_INV_DST_ALPHA, BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_ONE),
                                        BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_OP_FAIL_S_REPLACE | BGFX_STENCIL_OP_PASS_Z_REPLACE);

            _pop_nowarn_c_cast();
        });
    };
    EngineEvent::OnKeyHeld += [](Key key)
    {
        Entities::forEach<EntityAttributes, ShapeRendererTestComponent>([&](Entity& entity, EntityAttributes& attributes, ShapeRendererTestComponent&) -> void
        {
            _fence_value_return(void(), attributes.name != "Test Entity");
            inputTransformEntity(entity, key);
        });
    };
    EngineEvent::OnMouseScroll += [](glm::vec2 scroll)
    {
        Entities::forEach<EntityAttributes, ShapeRendererTestComponent>([&](Entity& entity, EntityAttributes& attributes, ShapeRendererTestComponent&) -> void
        {
            _fence_value_return(void(), attributes.name != "Test Entity");
            inputScaleEntity(entity, scroll);
        });
    };
    EngineEvent::OnMouseMove += [](glm::vec2 from)
    {
        Entities::forEach<EntityAttributes, ShapeRendererTestComponent>([&](Entity& entity, EntityAttributes& attributes, ShapeRendererTestComponent&) -> void
        {
            _fence_value_return(void(), attributes.name != "Test Entity");
            inputMoveEntity(entity, from);
        });
    };

    return 0;
}
