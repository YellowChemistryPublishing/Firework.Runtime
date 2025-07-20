#include "../common.h"

#include <Firework.Components.Core2D>
#include <Firework.Runtime.CoreLib>

namespace fs = std::filesystem;

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;
using namespace Firework::PackageSystem;

int main(int argc, char* argv[])
{
    std::error_code ec;
    fs::path curPath = fs::current_path(ec);
    if (!ec)
        (void)PackageManager::loadPackageIntoMemory(curPath / "Runtime" / "CorePackage.fwpkg");

    hookExampleControls();

    EngineEvent::OnInitialize += []
    {
        auto entity = Entity::alloc();
        entity->addComponent<EntityAttributes>()->name = "Test Entity";

        auto graphic = entity->addComponent<ScalableVectorGraphic>();
        graphic->svgFile = std::dynamic_pointer_cast<ExtensibleMarkupPackageFile>(PackageManager::lookupFileByPath(L"assets/tiger.svg"));

        Debug::printHierarchy();
    };
    EngineEvent::OnWindowResize += [](sysm::vector2i32)
    {
        Entities::forEach<EntityAttributes, ScalableVectorGraphic>([](Entity& entity, EntityAttributes& attributes, ScalableVectorGraphic& graphic) -> void
        {
            _fence_value_return(void(), attributes.name != "Test Entity");

            auto rectTransform = entity.getOrAddComponent<RectTransform>();
            rectTransform->rect =
                RectFloat(float(Window::pixelHeight()) / 2.0f, float(Window::pixelWidth()) / 2.0f, -float(Window::pixelHeight()) / 2.0f, -float(Window::pixelWidth()) / 2.0f);
        });
    };

    return 0;
}
