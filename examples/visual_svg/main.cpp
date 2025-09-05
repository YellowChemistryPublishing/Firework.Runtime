#include "../common.h"

#include <Firework.Components.Core2D>
#include <Firework.Runtime.CoreLib>

namespace fs = std::filesystem;

using namespace Firework;
using namespace Firework::PackageSystem;

int main(int, char*[])
{
    auto ec = std::error_code();
    auto curPath = fs::current_path(ec);
    if (!ec)
        (void)PackageManager::loadPackageIntoMemory(curPath / "Runtime" / "CorePackage.fwpkg");

    hookExampleControls();

    EngineEvent::OnInitialize += []
    {
        auto entity = Entity::alloc();
        entity->addComponent<EntityAttributes>()->name = "Test Entity";

        auto graphic = entity->addComponent<ScalableVectorGraphic>();
        graphic->svgFile = std::dynamic_pointer_cast<ExtensibleMarkupPackageFile>(PackageManager::lookupFileByPath(L"assets/tiger.svg"));

        auto rectTransform = entity->getOrAddComponent<RectTransform>();
        rectTransform->rect =
            RectFloat(float(Window::pixelHeight()) / 2.0f, float(Window::pixelWidth()) / 2.0f, -float(Window::pixelHeight()) / 2.0f, -float(Window::pixelWidth()) / 2.0f);

        Debug::printHierarchy();
    };
    EngineEvent::OnKeyHeld += [](Key key)
    {
        Entities::forEach<EntityAttributes, ScalableVectorGraphic>([&](Entity& entity, EntityAttributes& attributes, ScalableVectorGraphic&) -> void
        {
            _fence_value_return(void(), attributes.name != "Test Entity");
            inputTransformEntity(entity, key);
        });
    };
    EngineEvent::OnMouseScroll += [](glm::vec2 scroll)
    {
        Entities::forEach<EntityAttributes, ScalableVectorGraphic>([&](Entity& entity, EntityAttributes& attributes, ScalableVectorGraphic&) -> void
        {
            _fence_value_return(void(), attributes.name != "Test Entity");
            inputScaleEntity(entity, scroll);
        });
    };
    EngineEvent::OnMouseMove += [](glm::vec2 from)
    {
        Entities::forEach<EntityAttributes, ScalableVectorGraphic>([&](Entity& entity, EntityAttributes& attributes, ScalableVectorGraphic&) -> void
        {
            _fence_value_return(void(), attributes.name != "Test Entity");
            inputMoveEntity(entity, from);
        });
    };

    return 0;
}
