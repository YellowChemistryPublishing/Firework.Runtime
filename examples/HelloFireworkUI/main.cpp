#include <Firework.Core.hpp>
#include <Firework/Entry.h>

using namespace Firework;
using namespace Firework::PackageSystem;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    EngineEvent::OnInitialize += []
    {
    };
    EngineEvent::OnWindowResize += []([[maybe_unused]] sysm::vector2i32 from)
    {
    };

    return 0;
}