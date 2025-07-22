#include "Debug.h"

#include <Core/CoreEngine.h>
#include <EntityComponentSystem/Entity.h>
#include <GL/Renderer.h>
#include <SDL3/SDL.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;

void Debug::showF3Menu(bool visible)
{
    Renderer::showDebugInformation(visible);
}
void Debug::showWireframes(bool visible)
{
    Renderer::showDebugWireframes(visible);
}

void Debug::messageBox(LogLevel severity, std::string_view title, std::string_view message)
{
    SDL_MessageBoxFlags flags = [&]
    {
        switch (severity)
        {
        case LogLevel::Trace:
            return SDL_MESSAGEBOX_INFORMATION;
            break;
        case LogLevel::Info:
            return SDL_MESSAGEBOX_INFORMATION;
            break;
        case LogLevel::Warn:
            return SDL_MESSAGEBOX_WARNING;
            break;
        case LogLevel::Error:
        default:
            return SDL_MESSAGEBOX_ERROR;
            break;
        }
    }();

    SDL_ShowSimpleMessageBox(flags, title.data(), message.data(), CoreEngine::wind);
}
