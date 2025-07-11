#include "Application.h"

#include <Core/CoreEngine.h>
#define SDL_MAIN_NOIMPL
#include <SDL3/SDL_main.h>

using namespace Firework;
using namespace Firework::Internal;

moodycamel::ConcurrentQueue<func::function<void()>> Application::mainThreadQueue;
moodycamel::ConcurrentQueue<func::function<void()>> Application::workerThreadQueue;
moodycamel::ConcurrentQueue<func::function<void()>> Application::windowThreadQueue;

RuntimeInitializationOptions Application::_initializationOptions;
Property<const RuntimeInitializationOptions&, RuntimeInitializationOptions> Application::initializationOptions
{
    []() -> const RuntimeInitializationOptions& { return Application::_initializationOptions; },
    [](RuntimeInitializationOptions value) { Application::_initializationOptions = std::move(value); }
};

float Application::secondsPerFrame = 1.0f / 60.0f;

namespace Firework::Internal
{
    _fw_core_api int __fw_rt_fwd_main_invoc(int argc, char* argv[], SDL_main_func mainFunction, void* reserved)
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