#include "Display.h"

#include <Core/Application.h>
#include <Core/CoreEngine.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::Mathematics;

int Window::width;
int Window::height;
bool Window::resizing = false;

int Screen::width;
int Screen::height;
int Screen::screenRefreshRate;

bool Cursor::_visible = true;
Property<bool, bool> Cursor::visible
{
    []() -> bool
    {
        return Cursor::_visible;
    },
    [](bool value) -> void
    {
        Cursor::setVisible(value);
    }
};

CursorLockState Cursor::_lockState = CursorLockState::None;
Property<CursorLockState, CursorLockState> Cursor::lockState
{
    []() -> CursorLockState
    {
        return Cursor::_lockState;
    },
    [](CursorLockState value) -> void
    {
        Cursor::setLockState(value);
    }
};

void Window::setResolution(Vector2Int resolution)
{
    Application::queueJobForWindowThread([resolution]() -> void
    {
        // ```Window::width``` and ```Window::height``` are updated when the window resize event is handled.
        SDL_SetWindowSize(CoreEngine::wind, resolution.x, resolution.y);
    });
}

void Cursor::setVisible(bool value)
{
    Application::queueJobForWindowThread([value]() -> void
    {
        if (value)
            SDL_ShowCursor();
        else SDL_HideCursor();
    });
    Cursor::_visible = value;
}
void Cursor::setLockState(CursorLockState value)
{
    Application::queueJobForWindowThread([value, w = Window::width, h = Window::height]() -> void
    {
        switch (value)
        {
        case CursorLockState::None:
            SDL_SetWindowRelativeMouseMode(CoreEngine::wind, SDL_FALSE);
            SDL_SetWindowMouseGrab(CoreEngine::wind, SDL_FALSE);
            break;
        case CursorLockState::Hidden:
            SDL_SetWindowRelativeMouseMode(CoreEngine::wind, SDL_TRUE);
            SDL_SetWindowMouseGrab(CoreEngine::wind, SDL_FALSE);
            break;
        case CursorLockState::Confined:
            SDL_SetWindowRelativeMouseMode(CoreEngine::wind, SDL_FALSE);
            SDL_SetWindowMouseGrab(CoreEngine::wind, SDL_TRUE);
            break;
        }
    });
    Cursor::_lockState = value;
}
