#pragma once

#include <Firework.Runtime.CoreLib>

static void hookExampleControls()
{
    using namespace Firework;

    EngineEvent::OnKeyDown += [](Key key)
    {
        switch (key)
        {
        case Key::Function2:
            Debug::showWireframes();
            break;
        case Key::Function3:
            Debug::showF3Menu();
            break;
        }
    };
    EngineEvent::OnKeyUp += [](Key key)
    {
        switch (key)
        {
        case Key::Function2:
            Debug::hideWireframes();
            break;
        case Key::Function3:
            Debug::hideF3Menu();
            break;
        }
    };
}
