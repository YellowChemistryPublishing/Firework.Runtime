#include <filesystem>
#include <numbers>

#include <Components/EntityAttributes.h>
#include <Components/ScalableVectorGraphic.h>
#include <Components/Text.h>
#include <Core/CoreEngine.h>
#include <Core/PackageManager.h>
#include <Firework.Core.hpp>
#include <Firework/Entry.h>
#include <Friends/FilledPathRenderer.h>
#include <GL/Geometry.h>
#include <GL/Renderer.h>
#include <PackageSystem/ExtensibleMarkupFile.h>
#include <PackageSystem/PortableNetworkGraphicFile.h>
#include <PackageSystem/TrueTypeFontFile.h>

namespace fs = std::filesystem;

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;
using namespace Firework::PackageSystem;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::error_code ec;
    fs::path curPath = fs::current_path(ec);
    if (!ec)
        PackageManager::loadPackageIntoMemory(curPath / "Runtime" / "CorePackage.fwpkg");

    EngineEvent::OnInitialize += []
    {
        auto e = Entity::alloc();
        e->addComponent<EntityAttributes>()->name = "beans";
        auto rt = e->getOrAddComponent<RectTransform>();
        rt->rect = RectFloat(400, 400, -400, -400);
        // auto t = e->addComponent<Text>();
        // t->font = nullptr;
        // t->fontSize = 100;
        // t->text = U"1000000what";
        // t->fontSize = 4;
        // t->text = U"beans beans beans beans beans beans beans beans beans beans beans beans beans beans beans beans beans beans beans beans beans beans beans beans beans beans "
        //           U"beans beans beans beans beans beans beans beans beans";
        // t->font = std::dynamic_pointer_cast<TrueTypeFontPackageFile>(PackageManager::lookupFileByPath(L"assets/Comic Sans MS.ttf"));
        // t->fontSize = 200;
        rt->rect = RectFloat(720 / 2, 1280 / 2, -720 / 2, -1280 / 2);
        // rt->localRotation += std::numbers::pi_v<float> / 2.0f;
        auto s = e->addComponent<ScalableVectorGraphic>();
        s->svgFile = std::dynamic_pointer_cast<ExtensibleMarkupPackageFile>(PackageManager::lookupFileByPath(L"assets/tiger.svg"));

        auto e2 = Entity::alloc();
        e2->parent = e;
        auto e3 = Entity::alloc();

        Debug::printHierarchy();
        // Debug::showF3Menu();
    };

    return 0;
}
