#include "Display.h"

#include <Core/Application.h>
#include <Core/CoreEngine.h>

using namespace Firework;
using namespace Firework::Internal;

std::string Window::_name = "Program";
bool Window::_resizable = true;

glm::ivec2 Window::_minimumSize = glm::ivec2(200, 200);
i32 Window::width = 1280;
i32 Window::height = 720;
bool Window::resizing = false;

void Window::setName(std::string&& value)
{
    if (CoreEngine::state[size_t(EngineState::RenderInit)].test()) [[likely]]
        Application::queueJobForWindowThread([name = value]() -> void { SDL_SetWindowTitle(CoreEngine::wind, name.c_str()); });
    Window::_name = std::move(value);
}
void Window::setMinimumSize(glm::ivec2 value)
{
    if (CoreEngine::state[size_t(EngineState::RenderInit)].test()) [[likely]]
        Application::queueJobForWindowThread([]()
        { SDL_SetWindowMinimumSize(CoreEngine::wind, +sys::integer<int>(Window::_minimumSize.x), +sys::integer<int>(Window::_minimumSize.y)); });
    Window::_minimumSize = value;
}
void Window::setResizable(const bool value)
{
    if (CoreEngine::state[size_t(EngineState::RenderInit)].test()) [[likely]]
        Application::queueJobForWindowThread([value]() -> void { SDL_SetWindowResizable(CoreEngine::wind, value); });
    Window::_resizable = value;
}

void Window::setResolution(glm::i32vec2 resolution)
{
    Application::queueJobForWindowThread([resolution]() -> void
    {
        // ```Window::width``` and ```Window::height``` are updated when the window resize event is handled.
        SDL_SetWindowSize(CoreEngine::wind, +sys::integer<int>(resolution.x), +sys::integer<int>(resolution.y));
    });
}

i32 Screen::width;
i32 Screen::height;
i32 Screen::screenRefreshRate;
