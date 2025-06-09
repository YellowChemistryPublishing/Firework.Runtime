#include "EngineEvent.h"

#include <GL/Renderer.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::Mathematics;
using namespace Firework::GL;

FuncPtrEvent<> EngineEvent::OnInitialize;
FuncPtrEvent<> EngineEvent::OnTick;
FuncPtrEvent<> EngineEvent::OnLateTick;
FuncPtrEvent<> EngineEvent::OnPhysicsTick;

FuncPtrEvent<MouseButton> EngineEvent::OnMouseDown;
FuncPtrEvent<MouseButton> EngineEvent::OnMouseHeld;
FuncPtrEvent<MouseButton> EngineEvent::OnMouseUp;
FuncPtrEvent<Vector2Int> EngineEvent::OnMouseMove;
FuncPtrEvent<Vector2> EngineEvent::OnMouseScroll;

FuncPtrEvent<Key> EngineEvent::OnKeyDown;
FuncPtrEvent<Key> EngineEvent::OnKeyHeld;
FuncPtrEvent<Key> EngineEvent::OnKeyRepeat;
FuncPtrEvent<Key> EngineEvent::OnKeyUp;

FuncPtrEvent<const std::u32string&> EngineEvent::OnTextInput;

FuncPtrEvent<Vector2Int> EngineEvent::OnWindowResize;

FuncPtrEvent<> EngineEvent::OnAcquireFocus;
FuncPtrEvent<> EngineEvent::OnLoseFocus;

FuncPtrEvent<> EngineEvent::OnQuit;

// Runs in main thread.
FuncPtrEvent<Component2D*> InternalEngineEvent::OnRenderOffloadForComponent2D;
FuncPtrEvent<Component2D*> InternalEngineEvent::OnLateRenderOffloadForComponent2D;
FuncPtrEvent<Component*> InternalEngineEvent::OnRenderOffloadForComponent;
// Runs in render thread. Note FuncPtrEvent is not thread-safe,
// so make sure you modify this event _before_ the main thread loop exits.
FuncPtrEvent<> InternalEngineEvent::OnRenderShutdown;
