#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <string_view>
#include <Core/Input.h>
#include <Library/Event.h>

namespace Firework
{
    /// @brief Static class containing functionality relevant to runtime handled events. You may add as many callbacks as you wish.
    /// @warning You are **not** allowed to invoke any of this events yourself!
    class __firework_corelib_api EngineEvent final
    {
    public:
        EngineEvent() = delete;

        /// @brief Event raised immediately after the runtime has initialized.
        /// @note Main thread only.
        static FuncPtrEvent<> OnInitialize;
        /// @brief Event raised immediately before the runtime is shutdown.
        /// @note Main thread only.
        static FuncPtrEvent<> OnTick;
        /// @brief Event raised when physics increments.
        /// @warning Unimplemented.
        /// @note Physics thread only.
        static FuncPtrEvent<> OnPhysicsTick;

        /// @brief Event raised when a key is pressed down.
        /// @param key ```Firework::Key```
        /// @note Main thread only.
        static FuncPtrEvent<Key> OnKeyDown;
        /// @brief Event raised when a key remains held during the current frame.
        /// @param key ```Firework::Key```
        /// @note Main thread only.
        static FuncPtrEvent<Key> OnKeyHeld;
        /// @brief Event raised when a key repeat is triggered when a key is held down.
        /// @param key ```Firework::Key```
        /// @note Main thread only.
        static FuncPtrEvent<Key> OnKeyRepeat;
        /// @brief Event raised when a key is let go.
        /// @note Main thread only.
        static FuncPtrEvent<Key> OnKeyUp;
        /// @brief Event raised when a mouse button is pressed down.
        /// @param button ```Firework::MouseButton```
        /// @note Main thread only.
        static FuncPtrEvent<MouseButton> OnMouseDown;
        /// @brief Event raised when a mouse button remains held during the current frame.
        /// @param button ```Firework::MouseButton```
        /// @note Main thread only.
        static FuncPtrEvent<MouseButton> OnMouseHeld;
        /// @brief Event raised when a mouse button is let go.
        /// @param button ```Firework::MouseButton```
        /// @note Main thread only.
        static FuncPtrEvent<MouseButton> OnMouseUp;
        /// @brief Event raised when the cursor is moved.
        /// @param from ```Firework::Mathematics::Vector2Int```. The previous position of the cursor. Retrieve the current position of the cursor with ```Firework::Input::mousePosition()```.
        /// @note Main thread only.
        static FuncPtrEvent<Mathematics::Vector2Int> OnMouseMove;

        /// @brief Event raised when the window is resized.
        /// @param from ```Firework::Mathematics::Vector2Int```. The previous size of the window. Retrieve the current window size with ```Firework::Window::pixelWidth()``` and ```Firework::Window::pixelHeight()```.
        /// @note Main thread only.
        static FuncPtrEvent<Mathematics::Vector2Int> OnWindowResize;

        /// @brief Event raised when the window is brought into focus.
        /// @note Main thread only.
        static FuncPtrEvent<> OnAcquireFocus;
        /// @brief Event raised when the window is loses focus.
        /// @note Main thread only.
        static FuncPtrEvent<> OnLoseFocus;

        /// @brief
        /// Event raised immediately before the runtime is shutdown.
        /// You are guaranteed that the runtime has not destroyed existing scenes, entities, or components for cleanup yet.
        /// @note Main thread only.
        static FuncPtrEvent<> OnQuit;
    };

    namespace Internal
    {
        class Component2D;
        class Component;

        /// @brief Static class containing functionality relevant to internal/low-level runtime handled events. You may add as many callbacks as you wish.
        /// @warning You are **not** allowed to invoke any of this events yourself!
        class __firework_corelib_api InternalEngineEvent final
        {
        public:
            InternalEngineEvent() = delete;

            /// @internal
            /// @brief Low-level API. Event raised when a 2D component should be sent to render.
            /// @param component ```Firework::Internal::Component2D*```
            /// @note Main thread only.
            static FuncPtrEvent<Component2D*> OnRenderOffloadForComponent2D;
            /// @internal
            /// @brief Low-level API. Event raised when a 3D component should be sent to render.
            /// @param component ```Firework::Internal::Component*```
            /// @note Main thread only.
            static FuncPtrEvent<Component*> OnRenderOffloadForComponent;
            /// @internal
            /// @brief Low-level API. Event raised immediately before the render thread exits.
            /// @note Render thread only.
            static FuncPtrEvent<> OnRenderShutdown;
        };
    }
}