#include "EngineEvent.h"

#include <GL/Renderer.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;

FuncPtrEvent<> EngineEvent::OnInitialize;
FuncPtrEvent<> EngineEvent::OnTick;
FuncPtrEvent<> EngineEvent::OnLateTick;
FuncPtrEvent<> EngineEvent::OnPhysicsTick;

FuncPtrEvent<MouseButton> EngineEvent::OnMouseDown;
FuncPtrEvent<MouseButton> EngineEvent::OnMouseHeld;
FuncPtrEvent<MouseButton> EngineEvent::OnMouseUp;
FuncPtrEvent<sysm::vector2> EngineEvent::OnMouseMove;
FuncPtrEvent<sysm::vector2> EngineEvent::OnMouseScroll;

FuncPtrEvent<Key> EngineEvent::OnKeyDown;
FuncPtrEvent<Key> EngineEvent::OnKeyHeld;
FuncPtrEvent<Key> EngineEvent::OnKeyRepeat;
FuncPtrEvent<Key> EngineEvent::OnKeyUp;

FuncPtrEvent<const std::u32string&> EngineEvent::OnTextInput;

FuncPtrEvent<sysm::vector2i32> EngineEvent::OnWindowResize;

FuncPtrEvent<> EngineEvent::OnAcquireFocus;
FuncPtrEvent<> EngineEvent::OnLoseFocus;

FuncPtrEvent<> EngineEvent::OnQuit;

// Runs in main thread.
FuncPtrEvent<std::type_index, std::shared_ptr<void>> InternalEngineEvent::OnRenderOffloadForComponent;
// Runs in render thread. Note FuncPtrEvent is not thread-safe,
// so make sure you modify this event _before_ the main thread loop exits.
FuncPtrEvent<> InternalEngineEvent::OnRenderShutdown;
