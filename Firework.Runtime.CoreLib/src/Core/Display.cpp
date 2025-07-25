#include "Display.h"

#include <Core/Application.h>
#include <Core/CoreEngine.h>

using namespace Firework;
using namespace Firework::Internal;

i32 Window::width = 1280;
i32 Window::height = 720;
bool Window::resizing = false;

i32 Screen::width;
i32 Screen::height;
i32 Screen::screenRefreshRate;

void Window::setResolution(glm::i32vec2 resolution)
{
    Application::queueJobForWindowThread([resolution]() -> void
    {
        // ```Window::width``` and ```Window::height``` are updated when the window resize event is handled.
        SDL_SetWindowSize(CoreEngine::wind, +sys::integer<int>(resolution.x), +sys::integer<int>(resolution.y));
    });
}
