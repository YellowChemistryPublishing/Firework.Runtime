#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <SDL3/SDL.h>

#include <Mathematics.h>
#include <Library/Property.h>

namespace Firework
{
    namespace Internal
    {
        class CoreEngine;
    }

    class Cursor;

    enum class BuiltinCursorTexture
    {
        Default = SDL_SYSTEM_CURSOR_DEFAULT,
        Text = SDL_SYSTEM_CURSOR_TEXT,
        Wait = SDL_SYSTEM_CURSOR_WAIT, // Not responding texture - the spinning circle!
        Crosshair = SDL_SYSTEM_CURSOR_CROSSHAIR,
        Progress = SDL_SYSTEM_CURSOR_PROGRESS, // Spinning circle with arrow!
        BiArrowDiagonalDown = SDL_SYSTEM_CURSOR_NWSE_RESIZE,
        BiArrowDiagonalUp = SDL_SYSTEM_CURSOR_NESW_RESIZE,
        BiArrowHorizontal = SDL_SYSTEM_CURSOR_EW_RESIZE,
        BiArrowVertical = SDL_SYSTEM_CURSOR_NS_RESIZE,
        Move = SDL_SYSTEM_CURSOR_MOVE,
        No = SDL_SYSTEM_CURSOR_NOT_ALLOWED, // Red circle with a line through it!
        Pointer = SDL_SYSTEM_CURSOR_POINTER, // Hand cursor!
        ArrowTopLeft = SDL_SYSTEM_CURSOR_NW_RESIZE,
        ArrowUp = SDL_SYSTEM_CURSOR_N_RESIZE,
        ArrowUpRight = SDL_SYSTEM_CURSOR_NE_RESIZE,
        ArrowRight = SDL_SYSTEM_CURSOR_E_RESIZE,
        ArrowDownRight = SDL_SYSTEM_CURSOR_SE_RESIZE,
        ArrowDown = SDL_SYSTEM_CURSOR_S_RESIZE,
        ArrowDownLeft = SDL_SYSTEM_CURSOR_SW_RESIZE,
        ArrowLeft = SDL_SYSTEM_CURSOR_W_RESIZE
    };

    enum class CursorLockState
    {
        None,
        Hidden,
        Confined
    };

    struct __firework_corelib_api CursorTexture final
    {
        CursorTexture(BuiltinCursorTexture texture);
        ~CursorTexture();
    private:
        SDL_Cursor* internalCursor;
        
        friend class Firework::Cursor;
    };

    class __firework_corelib_api Cursor final
    {
        static bool _visible;
        static CursorLockState _lockState;

        static void setVisible(bool value);
        static void setLockState(CursorLockState value);
    public:
        static Property<bool, bool> visible;
        static Property<CursorLockState, CursorLockState> lockState;

        static void setCursor(CursorTexture* cursor);

        friend class Firework::Internal::CoreEngine;
    };
}