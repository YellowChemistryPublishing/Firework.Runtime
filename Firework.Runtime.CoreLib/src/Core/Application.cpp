#include "Application.h"

#define SDL_MAIN_NOIMPL
#include <SDL3/SDL_main.h>

#include <Core/CoreEngine.h>

using namespace Firework;
using namespace Firework::Internal;

moodycamel::ConcurrentQueue<func::function<void()>> Application::mainThreadQueue;
moodycamel::ConcurrentQueue<func::function<void()>> Application::windowThreadQueue;

std::atomic_flag Application::workerThreadQueueNotif = ATOMIC_FLAG_INIT;
moodycamel::ConcurrentQueue<func::function<void()>> Application::workerThreadQueue;

float Application::secondsPerFrame = 1.0f / 160.0f;

int Application::run(int argc, char* argv[])
{
    return CoreEngine::execute(argc, argv);
}
void Application::quit()
{
    CoreEngine::state[size_t(EngineState::ExitRequested)].test_and_set();
    CoreEngine::state[size_t(EngineState::ExitRequested)].notify_all();
}

namespace Firework::Internal
{
    _fw_core_api int _fw_rt_fwd_main_invoc(int argc, char* argv[], SDL_main_func mainFunction, void* reserved)
    {
        return SDL_RunApp(argc, argv, mainFunction, reserved);
    }
} // namespace Firework::Internal
