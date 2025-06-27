#include <Firework.Core.hpp>
#include <Firework/Entry.h>

using namespace Firework;
using namespace Firework::PackageSystem;

struct EntityAttributes
{
    std::string name;
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    EngineEvent::OnInitialize += []
    {
        auto e = std::make_shared<Entity>();
        e->addComponent<EntityAttributes>()->name = "beans";
        auto e2 = std::make_shared<Entity>();
        e2->addComponent<EntityAttributes>()->name = "beans2";
        auto e3 = std::make_shared<Entity>();
        e3->addComponent<EntityAttributes>()->name = "beans3";
        e3->parent = e.get();

        Entities::forEach<EntityAttributes>([&](Entity& entity, const EntityAttributes& attr) -> void
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
