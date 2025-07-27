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
        entity->addComponent<EntityAttributes>()->name = "Test Entity (Left-Align)";

        auto paragraph = entity->addComponent<Text>();
        paragraph->font = std::dynamic_pointer_cast<TrueTypeFontPackageFile>(PackageManager::lookupFileByPath(L"assets/Bagnard.otf"));
        paragraph->fontSize = 1000;
        paragraph->text = U"It is not the duty of the typographer to consciously display or emulate the style of current trends, not to reflect the spirit of the times.";
        paragraph->color = Color(0, 255, 255);

        auto rectTransform = entity->getOrAddComponent<RectTransform>();
        rectTransform->position = glm::vec2(0.0f, float(Window::pixelHeight()) / 2.0f);
        rectTransform->rect = RectFloat(0.0f, float(Window::pixelWidth()) / 2.0f, -720.0f, -float(Window::pixelWidth()) / 2.0f);

        Debug::printHierarchy();
    };
    EngineEvent::OnKeyHeld += [](Key key)
    {
        Entities::forEach<EntityAttributes, Text>([&](Entity& entity, EntityAttributes& attributes, Text&) -> void
        {
            _fence_value_return(void(), attributes.name != "Test Entity (Left-Align)");
            inputTransformEntity(entity, key);
        });
    };
    EngineEvent::OnMouseMove += [](glm::vec2 from)
    {
        Entities::forEach<EntityAttributes, Text>([&](Entity& entity, EntityAttributes& attributes, Text&) -> void
        {
            _fence_value_return(void(), attributes.name != "Test Entity (Left-Align)");
            inputMoveEntity(entity, from);
        });
    };

    return 0;
}
