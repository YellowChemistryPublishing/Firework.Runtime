#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <memory>
#include <string_view>
#include <typeindex>

#include <Core/Input.h>
#include <Library/Event.h>

namespace Firework
{
    class Entity;

    /// @brief Static class containing functionality relevant to runtime handled events. You may add as many callbacks as you wish.
    /// @warning You are **not** allowed to invoke any of this events yourself!
    class _fw_core_api EngineEvent final
    {
    public:
        EngineEvent() = delete;

        /// @brief Event raised immediately after the runtime has initialized.
        /// @note Main thread only.
        static FuncPtrEvent<> OnInitialize;
        /// @brief Event raised every logic frame.
        /// @note Main thread only.
        static FuncPtrEvent<> OnTick;
        /// @brief Event raised every logic frame, immediately after ```Firework::EngineEvent::OnTick```.
        /// @note Main thread only.
        static FuncPtrEvent<> OnLateTick;
        /// @brief Event raised when physics increments.
        /// @warning Unimplemented.
        /// @note Physics thread only.
        static FuncPtrEvent<> OnPhysicsTick;

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
        /// @param from ```sysm::vector2```. The previous position of the cursor. Retrieve the current position of the cursor with ```Firework::Input::mousePosition()```.
        /// @note Main thread only.
        static FuncPtrEvent<sysm::vector2> OnMouseMove;
        /// @brief Event raised when the mouse scrolls.
        /// @param scroll ```sysm::vector2```. The scroll amount in the x and y directions.
        /// @note Main thread only.
        static FuncPtrEvent<sysm::vector2> OnMouseScroll;

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

        /// @brief Event raised when text input is received.
        /// @param text ```const std::u32string&```.
        /// @note Main thread only.
        static FuncPtrEvent<const std::u32string&> OnTextInput;

        /// @brief Event raised when the window is resized.
        /// @param from ```sysm::vector2i32```. The previous size of the window. Retrieve the current window size with ```Firework::Window::pixelWidth()``` and
        /// ```Firework::Window::pixelHeight()```.
        /// @note Main thread only.
        static FuncPtrEvent<sysm::vector2i32> OnWindowResize;

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
        class _fw_core_api InternalEngineEvent final
        {
        public:
            InternalEngineEvent() = delete;

            /// @internal
            /// @note Main thread only.
            static FuncPtrEvent<std::type_index, Entity&, std::shared_ptr<void>, ssz> OnRenderOffloadForComponent;
            /// @internal
            /// @brief Low-level API. Event raised immediately before the render thread exits.
            /// @note Render thread only.
            static FuncPtrEvent<> OnRenderShutdown;
        };
    } // namespace Internal
} // namespace Firework