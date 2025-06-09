#include "Cursor.h"

#include <Core/Application.h>
#include <Core/CoreEngine.h>
#include <Core/Display.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::Mathematics;

CursorTexture::CursorTexture(BuiltinCursorTexture texture) :
internalCursor(SDL_CreateSystemCursor((SDL_SystemCursor)texture)) // Hope this is thread-safe lol.
{ }
CursorTexture::~CursorTexture()
{
    Application::queueJobForWindowThread([cursor = this->internalCursor]
    {
        SDL_DestroyCursor(cursor);
    });
}

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

void Cursor::setCursor(CursorTexture* texture)
{
    Application::queueJobForWindowThread([cursor = texture->internalCursor]
    {
        SDL_SetCursor(cursor);
    });
}
