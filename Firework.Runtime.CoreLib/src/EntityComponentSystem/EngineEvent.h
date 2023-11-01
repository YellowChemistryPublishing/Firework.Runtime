#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <string_view>
#include <Core/Input.h>
#include <Library/Event.h>

namespace Firework
{
    /// @brief Static class containing functionality relevant to runtime-dispatched events. You may add as many callbacks as you wish.
    class __firework_corelib_api EngineEvent final
    {
    public:
        EngineEvent() = delete;
        
        static FuncPtrEvent<> OnInitialize;
        static FuncPtrEvent<> OnTick;
        static FuncPtrEvent<> OnPhysicsTick;

        static FuncPtrEvent<Key> OnKeyDown;
        static FuncPtrEvent<Key> OnKeyHeld;
        static FuncPtrEvent<Key> OnKeyRepeat;
        static FuncPtrEvent<Key> OnKeyUp;
        static FuncPtrEvent<MouseButton> OnMouseDown;
        static FuncPtrEvent<MouseButton> OnMouseHeld;
        static FuncPtrEvent<MouseButton> OnMouseUp;
        static FuncPtrEvent<Mathematics::Vector2Int> OnMouseMove;

        static FuncPtrEvent<Mathematics::Vector2Int> OnWindowResize;

        static FuncPtrEvent<> OnAcquireFocus;
        static FuncPtrEvent<> OnLoseFocus;

        static FuncPtrEvent<> OnQuit;
    };

    namespace Internal
    {
        class Component2D;
        class Component;

        class __firework_corelib_api InternalEngineEvent final
        {
        public:
            static FuncPtrEvent<Component2D*> OnRenderOffloadForComponent2D;
            static FuncPtrEvent<Component*> OnRenderOffloadForComponent;
            static FuncPtrEvent<> OnRenderShutdown;
        };
    }
}