#include "Application.h"

#include <Core/CoreEngine.h>
#define SDL_MAIN_NOIMPL
#include <SDL3/SDL_main.h>

using namespace Firework;
using namespace Firework::Internal;

moodycamel::ConcurrentQueue<func::function<void()>> Application::mainThreadQueue;
moodycamel::ConcurrentQueue<func::function<void()>> Application::workerThreadQueue;
moodycamel::ConcurrentQueue<func::function<void()>> Application::windowThreadQueue;

float Application::secondsPerFrame = 1.0f / 60.0f;

namespace Firework::Internal
{
    __firework_corelib_api int __fw_rt_fwd_main_invoc(int argc, char* argv[], SDL_main_func mainFunction, void* reserved)
    {
        return SDL_RunApp(argc, argv, mainFunction, reserved);
    }
}

int Application::run(int argc, char* argv[])
{
    return CoreEngine::execute(argc, argv);
}
void Application::quit()
{
    CoreEngine::state.store(EngineState::ExitRequested, std::memory_order_relaxed);
}