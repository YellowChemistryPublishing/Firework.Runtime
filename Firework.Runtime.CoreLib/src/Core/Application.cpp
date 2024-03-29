#include "Application.h"

#include <Core/CoreEngine.h>

using namespace Firework;
using namespace Firework::Internal;

moodycamel::ConcurrentQueue<func::function<void()>> Application::mainThreadQueue;
float Application::secondsPerFrame = 1.0f / 60.0f;

int Application::run(int argc, char* argv[])
{
    return CoreEngine::execute(argc, argv);
}
void Application::quit()
{
    CoreEngine::state.store(EngineState::ExitRequested, std::memory_order_relaxed);
}