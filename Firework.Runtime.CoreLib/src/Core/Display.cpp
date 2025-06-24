#include "Display.h"

#include <Core/Application.h>
#include <Core/CoreEngine.h>

using namespace Firework;
using namespace Firework::Internal;

int Window::width;
int Window::height;
bool Window::resizing = false;

int Screen::width;
int Screen::height;
int Screen::screenRefreshRate;

void Window::setResolution(Vector2Int resolution)
{
    Application::queueJobForWindowThread([resolution]() -> void
    {
        // ```Window::width``` and ```Window::height``` are updated when the window resize event is handled.
        SDL_SetWindowSize(CoreEngine::wind, resolution.x, resolution.y);
    });
}
