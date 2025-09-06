#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

_push_nowarn_c_cast();
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <glm/vec2.hpp>
#include <queue>
#include <robin_hood.h>
#include <vector>
_pop_nowarn_c_cast();

namespace Firework::Internal
{
    class CoreEngine;
}

_push_nowarn_msvc(_clWarn_msvc_export_interface);
namespace Firework
{
    /// @brief Mouse buttons.
    enum class MouseButton : uint_fast8_t
    {
        Left = 0,
        Middle,
        Right,
        Special1,
        Special2,
        Count
    };
    /// @brief Keyboard keys.
    enum class Key : uint_fast16_t
    {
        Unknown = 0,

        LetterA,
        LetterB,
        LetterC,
        LetterD,
        LetterE,
        LetterF,
        LetterG,
        LetterH,
        LetterI,
        LetterJ,
        LetterK,
        LetterL,
        LetterM,
        LetterN,
        LetterO,
        LetterP,
        LetterQ,
        LetterR,
        LetterS,
        LetterT,
        LetterU,
        LetterV,
        LetterW,
        LetterX,
        LetterY,
        LetterZ,

        Number1,
        Number2,
        Number3,
        Number4,
        Number5,
        Number6,
        Number7,
        Number8,
        Number9,
        Number0,

        Return,
        SecondaryReturn,
        Escape,
        CapsLock,
        Space,
        Tab,
        Backspace,

        Period,
        Comma,
        ExclamationMark,
        QuestionMark,
        AtSign,
        Hashtag,
        Dollar,
        Percent,
        Caret,
        Ampersand,
        Asterisk,
        Colon,
        SemiColon,
        Underscore,

        Plus,
        Minus,
        Equals,
        GreaterThan,
        LessThan,

        SingleQuote,
        DoubleQuotes,
        BackQuote,

        LeftParenthesis,
        RightParenthesis,
        LeftSquareBracket,
        RightSquareBracket,
        ForwardSlash,
        BackSlash,

        Function1,
        Function2,
        Function3,
        Function4,
        Function5,
        Function6,
        Function7,
        Function8,
        Function9,
        Function10,
        Function11,
        Function12,
        Function13,
        Function14,
        Function15,
        Function16,
        Function17,
        Function18,
        Function19,
        Function20,
        Function21,
        Function22,
        Function23,
        Function24,

        PrintScreen,
        NumLock,
        ScrollLock,
        Home,
        Pause,
        End,
        Insert,
        Delete,
        PageUp,
        PageDown,

        UpArrow,
        DownArrow,
        RightArrow,
        LeftArrow,

        Numpad1,
        Numpad2,
        Numpad3,
        Numpad4,
        Numpad5,
        Numpad6,
        Numpad7,
        Numpad8,
        Numpad9,
        Numpad0,
        Numpad00,
        Numpad000,

        NumpadA,
        NumpadB,
        NumpadC,
        NumpadD,
        NumpadE,
        NumpadF,

        NumpadEnter,
        NumpadPeriod,
        NumpadComma,

        NumpadLeftParenthesis,
        NumpadRightParenthesis,
        NumpadLeftBrace,
        NumpadRightBrace,

        NumpadPlus,
        NumpadMinus,
        NumpadMultiply,
        NumpadDivide,
        NumpadEquals,
        NumpadEqualsAS400,
        NumpadLessThan,
        NumpadGreaterThan,

        Application /* On Windows, its the Windows key. */,
        Power,
        Menu,
        Help,

        Undo /* Again */,
        Redo,
        Cut,
        Copy,
        Paste,

        Select,
        Find,
        Prior,
        Clear,
        ClearAgain,
        Cancel,
        Stop,

        Execute,
        EraseEaze,
        SystemRequest,
        Separator,
        Out,
        Oper,
        CrSel,
        ExSel,

        MuteVolume,
        IncreaseVolume,
        DecreaseVolume,

        ThousandsSeparator,
        DecimalSeparator,
        CurrencyUnit,
        CurrencySubUnit,

        NumpadPower,
        NumpadSpace,
        NumpadTab,
        NumpadBackspace,
        NumpadExclamationMark,
        NumpadAtSign,
        NumpadHashtag,
        NumpadPercent,
        NumpadAmpersand,
        NumpadDoubleAmpersand,
        NumpadVerticalBar,
        NumpadDoubleVerticalBar,
        NumpadClear,
        NumpadClearEntry,
        NumpadBinary,
        NumpadOctal,
        NumpadDecimal,
        NumpadHexadecimal,
        NumpadColon,
        NumpadXor,

        NumpadMemAdd,
        NumpadMemSubtract,
        NumpadMemMultiply,
        NumpadMemDivide,
        NumpadPlusMinus,
        NumpadMemStore,
        NumpadMemRecall,
        NumpadMemClear,

        Control,
        LeftControl,
        RightControl,
        Shift,
        LeftShift,
        RightShift,
        Alt,
        LeftAlt,
        RightAlt,
        GUI,
        LeftGUI,
        RightGUI,

        ModeSwitch,

        MediaSelect,
        NextMedia,
        PreviousMedia,
        PlayMedia,
        StopMedia,
        FastForwardMedia,
        RewindMedia,
        MuteMedia,

        Computer,
        WorldWideWeb,
        Calculator,
        Mail,

        AppCtrlKeypadHome,
        AppCtrlKeypadSearch,
        AppCtrlKeypadForward,
        AppCtrlKeypadBack,
        AppCtrlKeypadStop,
        AppCtrlKeypadRefresh,
        AppCtrlKeypadBookmarks,

        IncreaseBrightness,
        DecreaseBrightness,
        IncreaseKeyboardIlluminationToggle,
        DecreaseKeyboardIllumination,
        KeyboardIlluminationToggle,

        DisplaySwitch,
        Eject,
        Sleep,

        App1,
        App2,

        Count
    };

    /// @brief Static class containing functionality relevant to input processing.
    class _fw_core_api Input final
    {
        static glm::vec2 internalMousePosition;
        static glm::vec2 internalMouseMotion;

        static bool heldMouseInputs[size_t(MouseButton::Count)];
        static bool heldKeyInputs[size_t(Key::Count)];

        /// @internal
        /// @brief Internal API. SDL mouse code to Firework::MouseButton
        /// @warning This won't return a valid Firework::MouseButton if you don't pass it a valid SDL_BUTTON_*.
        /// @param code Button to convert. From SDL_BUTTON_*.
        /// @return Firework::MouseButton representing the mouse button.
        /// @note Thread-safe.
        static MouseButton convertFromSDLMouse(uint_fast8_t code);
        /// @internal
        /// @brief Internal API. SDL key code to Firework::Key.
        /// @param code SDL_Keycode to convert.
        /// @return Firework::Key representing the keyboard key.
        /// @note Thread-safe.
        static Key convertFromSDLKey(SDL_Keycode code);
    public:
        /// @brief Retrieve the mouse position in pixel units, with the centre of the Window as (0, 0).
        /// @return Mouse position.
        /// @note Main thread only.
        inline static glm::vec2 mousePosition()
        {
            return Input::internalMousePosition;
        }
        /// @brief Retrieve the mouse motion this frame in pixel units.
        /// @return Amount mouse has moved this frame.
        /// @note Main thread only.
        inline static glm::vec2 mouseMotion()
        {
            return Input::internalMouseMotion;
        }

        /// @brief Retrieve whether mouse button is pressed this frame.
        /// @param button Mouse button to check.
        /// @return Whether button is pressed.
        /// @note Main thread only.
        inline static bool mouseHeld(MouseButton button)
        {
            return Input::heldMouseInputs[size_t(button)];
        }
        /// @brief Retrieve whether key is pressed this frame.
        /// @param key Key to check.
        /// @return Whether key is pressed.
        /// @note Main thread only.
        inline static bool keyHeld(Key key)
        {
            return Input::heldKeyInputs[size_t(key)];
        }

        static void beginQueryTextInput();
        static void endQueryTextInput();

        friend class Firework::Internal::CoreEngine;
    };
} // namespace Firework
_pop_nowarn_msvc();
