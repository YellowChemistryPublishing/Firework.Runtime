#include <Firework.Core.hpp>
#include <Firework/Entry.h>
#include <Components/EntityAttributes.h>

using namespace Firework;
using namespace Firework::PackageSystem;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    EngineEvent::OnInitialize += []
    {
        auto e = (new Entity())->shared_from_this();
        e->addComponent<EntityAttributes>()->name = "beans";
        (new Entity())->addComponent<EntityAttributes>()->name = "beans2";
        auto e3 = (new Entity())->shared_from_this();
        e3->addComponent<EntityAttributes>()->name = "beans3";
        e3->parent = e;

        Debug::printHierarchy();

        Entities::forEach<EntityAttributes>([&](Entity&, const EntityAttributes& attr) -> void
        {
            std::cout << attr.name << '\n';
        });

        Application::quit();
    };
    EngineEvent::OnWindowResize += []([[maybe_unused]] sysm::vector2i32 from)
    {
    };

    return 0;
}
