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

FuncPtrEvent<Key> EngineEvent::OnKeyDown;
FuncPtrEvent<Key> EngineEvent::OnKeyHeld;
FuncPtrEvent<Key> EngineEvent::OnKeyRepeat;
FuncPtrEvent<Key> EngineEvent::OnKeyUp;
FuncPtrEvent<MouseButton> EngineEvent::OnMouseDown;
FuncPtrEvent<MouseButton> EngineEvent::OnMouseHeld;
FuncPtrEvent<MouseButton> EngineEvent::OnMouseUp;
FuncPtrEvent<Vector2Int> EngineEvent::OnMouseMove;
FuncPtrEvent<Vector2> EngineEvent::OnMouseScroll;

FuncPtrEvent<Vector2Int> EngineEvent::OnWindowResize;

FuncPtrEvent<> EngineEvent::OnAcquireFocus;
FuncPtrEvent<> EngineEvent::OnLoseFocus;

FuncPtrEvent<> EngineEvent::OnQuit;

// Runs in main thread.
FuncPtrEvent<Component2D*> InternalEngineEvent::OnRenderOffloadForComponent2D;
FuncPtrEvent<Component*> InternalEngineEvent::OnRenderOffloadForComponent;
void (*InternalEngineEvent::ClearViewArea)() = []
{
    Renderer::setViewClear(0, 0x00000000, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL);
};
void (*InternalEngineEvent::ResetViewArea)(uint16_t, uint16_t) = [](uint16_t w, uint16_t h)
{
    Renderer::setViewArea(0, 0, 0, w, h);
    Renderer::setViewArea(1, 0, 0, w, h);
    Renderer::setViewOrthographic(1, w, h, Vector3(0, 0, 0), Renderer::fromEuler(Vector3(0, 0, 0)), 0.0f, 16777216.0f);
};
void (*InternalEngineEvent::ResetBackbuffer)(uint32_t, uint32_t) = [](uint32_t w, uint32_t h)
{
    Renderer::resetBackbuffer(w, h);
};
// Runs in render thread. Note FuncPtrEvent is not thread-safe,
// so make sure you modify this event _before_ the main thread loop exits.
void (*InternalEngineEvent::RenderFrame)() = &Renderer::drawFrame;
FuncPtrEvent<> InternalEngineEvent::OnRenderShutdown;
