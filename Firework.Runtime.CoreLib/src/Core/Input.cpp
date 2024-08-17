#include "Input.h"

#include <Core/Application.h>
#include <Core/CoreEngine.h>
#include <EntityComponentSystem/EngineEvent.h>

using namespace Firework;
using namespace Firework::Internal;

MouseButton Input::convertFromSDLMouse(uint_fast8_t code)
{
	return (MouseButton)(code - 1);
}
Key Input::convertFromSDLKey(SDL_Keycode code)
{
	switch (code)
	{
	case SDLK_A: return Key::LetterA;
	case SDLK_B: return Key::LetterB;
	case SDLK_C: return Key::LetterC;
	case SDLK_D: return Key::LetterD;
	case SDLK_E: return Key::LetterE;
	case SDLK_F: return Key::LetterF;
	case SDLK_G: return Key::LetterG;
	case SDLK_H: return Key::LetterH;
	case SDLK_I: return Key::LetterI;
	case SDLK_J: return Key::LetterJ;
	case SDLK_K: return Key::LetterK;
	case SDLK_L: return Key::LetterL;
	case SDLK_M: return Key::LetterM;
	case SDLK_N: return Key::LetterN;
	case SDLK_O: return Key::LetterO;
	case SDLK_P: return Key::LetterP;
	case SDLK_Q: return Key::LetterQ;
	case SDLK_R: return Key::LetterR;
	case SDLK_S: return Key::LetterS;
	case SDLK_T: return Key::LetterT;
	case SDLK_U: return Key::LetterU;
	case SDLK_V: return Key::LetterV;
	case SDLK_W: return Key::LetterW;
	case SDLK_X: return Key::LetterX;
	case SDLK_Y: return Key::LetterY;
	case SDLK_Z: return Key::LetterZ;

	case SDLK_1: return Key::Number1;
	case SDLK_2: return Key::Number2;
	case SDLK_3: return Key::Number3;
	case SDLK_4: return Key::Number4;
	case SDLK_5: return Key::Number5;
	case SDLK_6: return Key::Number6;
	case SDLK_7: return Key::Number7;
	case SDLK_8: return Key::Number8;
	case SDLK_9: return Key::Number9;
	case SDLK_0: return Key::Number0;

	case SDLK_RETURN: return Key::Return;
	case SDLK_RETURN2: return Key::SecondaryReturn;
	case SDLK_ESCAPE: return Key::Escape;
	case SDLK_CAPSLOCK: return Key::CapsLock;
	case SDLK_SPACE: return Key::Space;
	case SDLK_TAB: return Key::Tab;
	case SDLK_BACKSPACE: return Key::Backspace;

	case SDLK_PERIOD: return Key::Period;
	case SDLK_COMMA: return Key::Comma;
	case SDLK_EXCLAIM: return Key::ExclamationMark;
	case SDLK_QUESTION: return Key::QuestionMark;
	case SDLK_AT: return Key::AtSign;
	case SDLK_HASH: return Key::Hashtag;
	case SDLK_DOLLAR: return Key::Dollar;
	case SDLK_PERCENT: return Key::Percent;
	case SDLK_CARET: return Key::Caret;
	case SDLK_AMPERSAND: return Key::Ampersand;
	case SDLK_ASTERISK: return Key::Asterisk;
	case SDLK_COLON: return Key::Colon;
	case SDLK_SEMICOLON: return Key::SemiColon;
	case SDLK_UNDERSCORE: return Key::Underscore;

	case SDLK_PLUS: return Key::Plus;
	case SDLK_MINUS: return Key::Minus;
	case SDLK_EQUALS: return Key::Equals;
	case SDLK_GREATER: return Key::GreaterThan;
	case SDLK_LESS: return Key::LessThan;

	case SDLK_APOSTROPHE: return Key::SingleQuote;
	case SDLK_DBLAPOSTROPHE: return Key::DoubleQuotes;
	case SDLK_GRAVE: return Key::BackQuote;

	case SDLK_UP: return Key::UpArrow;
	case SDLK_DOWN: return Key::DownArrow;
	case SDLK_LEFT: return Key::LeftArrow;
	case SDLK_RIGHT: return Key::RightArrow;

	// TODO: Implement rest.
	
	case SDLK_UNKNOWN:
	default:
		return Key::Unknown;
	}
}

Mathematics::Vector2Int Input::internalMousePosition;
Mathematics::Vector2Int Input::internalMouseMotion;

bool Input::heldMouseInputs[(size_t)MouseButton::Count] { false };
bool Input::heldKeyInputs[(size_t)Key::Count] { false };

void Input::beginQueryTextInput()
{
	Application::queueJobForWindowThread([]
	{
		SDL_StartTextInput(CoreEngine::wind);
	});
}
void Input::endQueryTextInput()
{
	Application::queueJobForWindowThread([]
	{
		SDL_StopTextInput(CoreEngine::wind);
	});
}