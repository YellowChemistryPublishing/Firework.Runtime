#include "CoreEngine.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>
#if __has_include(<cpptrace/cpptrace.hpp>)
#include <cpptrace/cpptrace.hpp>
#endif
#include <csignal>
#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <numeric>
#if __linux__
#include <pthread.h>
#endif
#include <set>
#include <span>
#if _WIN32
#define NOMINMAX 1
// clang-format off: Order is important.
#include <windows.h>
#include <VersionHelpers.h>
// clang-format on
#endif

#include <Components/RectTransform.h>
#include <Core/Application.h>
#include <Core/Debug.h>
#include <Core/Display.h>
#include <Core/HardwareExcept.h>
#include <Core/Input.h>
#include <Core/PackageManager.h>
#include <Core/Time.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <EntityComponentSystem/Entity.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <Firework/Config.h>
#include <GL/RenderPipeline.h>
#include <GL/Renderer.h>

namespace fs = std::filesystem;
using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;
using namespace Firework::PackageSystem;

SDL_Window* CoreEngine::wind = nullptr;
SDL_Renderer* CoreEngine::rend = nullptr;
const SDL_DisplayMode* CoreEngine::displMd = nullptr;

std::atomic_flag CoreEngine::state[size_t(EngineState::Count)] {};

std::deque<RenderJob> CoreEngine::renderQueue;
std::mutex CoreEngine::renderQueueLock;
std::atomic<uint_fast8_t> CoreEngine::framesInFlight = 0;

int CoreEngine::execute(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

#if _WIN32 // Windows ANSI Escape Sequence Support
    if (IsWindowsVistaOrGreater())
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
        SetConsoleMode(hOut, dwMode);
    }
#endif

    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_HAPTIC | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK))
    {
        Debug::logError("Runtime failed to initialize! Error: ", SDL_GetError(), '.');
        return EXIT_FAILURE;
    }

    std::thread workerThread([]
    {
        func::function<void()> event;
        while (!CoreEngine::state[size_t(EngineState::WindowThreadDone)].test())
        {
            Application::workerThreadQueueNotif.wait(false);
            while (Application::workerThreadQueue.try_dequeue(event)) event();
            Application::workerThreadQueueNotif.clear();
        }
    });

    {
        std::jthread windowThread(internalWindowLoop);
        std::jthread renderThread(internalRenderLoop);
        std::jthread mainThread(internalLoop);
    }

    Application::workerThreadQueueNotif.test_and_set();
    Application::workerThreadQueueNotif.notify_one();
    workerThread.join();

    // Cleanup here is done for stuff created in CoreEngine::execute, thread-specific cleanup is done per-thread, at the end of their lifetime.

    SDL_Quit();

    return EXIT_SUCCESS;
}

void CoreEngine::resetDisplayData()
{
    if (!(CoreEngine::displMd = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay())))
    {
        Debug::logError("Failed to get desktop display mode: ", SDL_GetError());
        return;
    }

    if (CoreEngine::displMd) [[likely]]
    {
        Application::queueJobForMainThread([w = CoreEngine::displMd->w, h = CoreEngine::displMd->h, rr = CoreEngine::displMd->refresh_rate]
        {
            Screen::width = w;
            Screen::height = h;
            Screen::screenRefreshRate = rr;
        });
    }
}

constexpr static auto userFunctionInvoker = [](auto&& func)
{
    auto fmtTrace = [](cpptrace::stacktrace ret) -> std::string
    {
        for (auto& frame : ret.frames)
        {
            if (frame.symbol.empty())
                frame.symbol = "[optimized out]";
            if (frame.filename.empty())
                frame.filename = "[Unknown / JIT Compiled Code]";
        }
        return ret.to_string();
    };

    __hwTry
    {
        func();
    }
    __hwCatch(const cpptrace::exception_with_message& ex)
    {
        std::string traceback = "std::exception (";
        traceback.append(cpptrace::demangle(typeid(ex).name())).append("): ").append(ex.message()).append("\nUnhandled exception thrown, at:\n").append(fmtTrace(ex.trace()));
        Debug::logError(traceback);
    }
    __hwCatch(const cpptrace::exception& ex)
    {
        std::string traceback = "std::exception (";
        traceback.append(cpptrace::demangle(typeid(ex).name())).append("): ").append(ex.what()).append("\nUnhandled exception thrown, at:\n").append(fmtTrace(ex.trace()));
        Debug::logError(traceback);
    }
    __hwCatch(const Exception& ex)
    {
        std::string traceback = "std::exception (";
        traceback.append(cpptrace::demangle(typeid(ex).name())).append("): ").append(ex.what()).append("\nUnhandled exception thrown, at:\n");
        Debug::logError(traceback);
    }
    __hwCatch(const std::exception& ex)
    {
        std::string traceback = "std::exception (";
        traceback.append(cpptrace::demangle(typeid(ex).name())).append("): ").append(ex.what());
        traceback.append("\nUnhandled exception thrown, at:\n").append(fmtTrace(cpptrace::stacktrace::current()));
        Debug::logError(traceback);
    }
    __hwCatch(...)
    {
        std::string traceback = cpptrace::demangle(std::current_exception().__cxa_exception_type()->name());
        traceback.append(": ");
        traceback.append("[Unknown / JIT Compiled Code]");
        traceback.append("\nUnhandled exception thrown, at:\n").append(fmtTrace(cpptrace::stacktrace::current()));
        Debug::logError(traceback);
    }
    __hwEnd();
};

void CoreEngine::internalLoop()
{
    CoreEngine::state[size_t(EngineState::WindowInit)].test_and_set(); // Spin off window handling thread.
    CoreEngine::state[size_t(EngineState::WindowInit)].notify_all();

    CoreEngine::state[size_t(EngineState::RenderThreadReady)].wait(false, std::memory_order_relaxed);

    CoreEngine::state[size_t(EngineState::Running)].test_and_set();
    CoreEngine::state[size_t(EngineState::Running)].notify_all();

    // IMPORTANT: A userlevel function block needs to be guarded with `userFunctionInvoker(...)` to catch exceptions and prevent crashing the engine.
    userFunctionInvoker(EngineEvent::OnInitialize);
    // IMPORTANT END

    u64 frameBegin = SDL_GetPerformanceCounter();
    u64 perfFreq = SDL_GetPerformanceFrequency();

    float targetDeltaTime = Application::secondsPerFrame;
    float deltaTime = 0.0f;

    float prevw = float(+Window::width), prevh = float(+Window::height);

    // IMPORTANT: A userlevel function block needs to be guarded with `userFunctionInvoker(...)` to catch exceptions and prevent crashing the engine.
    userFunctionInvoker([] { EngineEvent::OnWindowResize(glm::i32vec2 { Window::width, Window::height }); });
    // IMPORTANT END

    func::function<void()> job;
    while (!CoreEngine::state[size_t(EngineState::ExitRequested)].test())
    {
        deltaTime = float(SDL_GetPerformanceCounter() - frameBegin) / float(perfFreq);
        if (Application::mainThreadQueue.try_dequeue(job))
            job();
        else if (deltaTime >= targetDeltaTime)
        {
#pragma region Input Events!
            for (uint_fast16_t i = 0; i < uint_fast16_t(MouseButton::Count); i++)
            {
                if (Input::heldMouseInputs[i])
                {
                    // IMPORTANT: A userlevel function block needs to be guarded with `userFunctionInvoker(...)` to catch exceptions and prevent crashing the engine.
                    userFunctionInvoker([&i] { EngineEvent::OnMouseHeld(MouseButton(i)); });
                    // IMPORTANT END
                }
            }
            for (uint_fast16_t i = 0; i < uint_fast16_t(Key::Count); i++)
            {
                if (Input::heldKeyInputs[i])
                {
                    // IMPORTANT: A userlevel function block needs to be guarded with `userFunctionInvoker(...)` to catch exceptions and prevent crashing the engine.
                    userFunctionInvoker([&i] { EngineEvent::OnKeyHeld(Key(i)); });
                    // IMPORTANT END
                }
            }
#pragma endregion

#pragma region Update RectTransform Components with Anchors
            for (Entity& entity : Entities::range())
            {
                if (std::shared_ptr<RectTransform> rectTransform = entity.getComponent<RectTransform>(); rectTransform && (Window::width != prevw || Window::height != prevh))
                {
                    RectFloat rectAnchor = rectTransform->rectAnchor();
                    RectFloat delta((Window::height - prevh) * 0.5f, (Window::width - prevw) * 0.5f, (Window::height - prevh) * -0.5f, (Window::width - prevw) * -0.5f);
                    if (rectAnchor != RectFloat(0.0f))
                        rectTransform->rect += delta * rectAnchor;

                    RectFloat positionAnchor = rectTransform->positionAnchor();
                    if (positionAnchor != RectFloat(0.0f))
                        rectTransform->position += glm::vec2(delta.right, delta.top) * glm::vec2(positionAnchor.right, positionAnchor.top) +
                            glm::vec2(delta.left, delta.bottom) * glm::vec2(positionAnchor.left, positionAnchor.bottom);
                }
            }
#pragma endregion

            // IMPORTANT: A userlevel function block needs to be guarded with `userFunctionInvoker(...)` to catch exceptions and prevent crashing the engine.
            userFunctionInvoker(EngineEvent::OnTick);
            userFunctionInvoker(EngineEvent::OnLateTick);
            // IMPORTANT END

#pragma region Render Offload
            // My code is held together with glue and duct tape. And not the good stuff either.

            if (Window::width != prevw || Window::height != prevh)
            {
                CoreEngine::queueRenderJobForFrame([w = Window::width, h = Window::height]
                {
                    RenderPipeline::resetBackbuffer(+u32(w), +u32(h));
                    RenderPipeline::resetViewArea(+u16(w), +u16(h));
                });
            }

            // This is a trade-off between stutter and microstutter. Higher values reduce stutter but increase latency and microstutter, lower values do the opposite.
            if (CoreEngine::framesInFlight < Config::MaxFramesInFlight) // Don't let the CPU get too far ahead of the GPU.
            {
                CoreEngine::queueRenderJobForFrame([]
                {
                    CoreEngine::framesInFlight++;
                    RenderPipeline::clearViewArea();
                });

                ssz renderIndex = 0;
                Entities::forEachEntity([&](Entity& entity)
                {
                    for (auto& [typeIndex, componentSet] : Entities::table)
                    {
                        auto componentIt = componentSet.find(&entity);
                        if (componentIt != componentSet.end())
                            userFunctionInvoker([&] { InternalEngineEvent::OnRenderOffloadForComponent(typeIndex, entity, componentIt->second, renderIndex++); });
                    }
                });
                Entities::forEachEntityReversed([&](Entity& entity)
                {
                    for (auto& [typeIndex, componentSet] : Entities::table)
                    {
                        auto componentIt = componentSet.find(&entity);
                        if (componentIt != componentSet.end())
                            userFunctionInvoker([&] { InternalEngineEvent::OnLateRenderOffloadForComponent(typeIndex, entity, componentIt->second, --renderIndex); });
                    }
                });

                CoreEngine::queueRenderJobForFrame([]
                {
                    RenderPipeline::renderFrame();
                    CoreEngine::framesInFlight--;
                });
            }
#pragma endregion

            Input::internalMouseMotion = glm::vec2(0.0f);

            prevw = float(+Window::width);
            prevh = float(+Window::height);

            frameBegin = SDL_GetPerformanceCounter();
            targetDeltaTime = std::max(Application::secondsPerFrame - (deltaTime - targetDeltaTime), 0.0f);
            Time::frameDeltaTime = deltaTime * Time::timeScale;
        }
        else
            CoreEngine::waitSome(std::chrono::nanoseconds(uint64_t((targetDeltaTime - (float(SDL_GetPerformanceCounter() - frameBegin) / float(perfFreq))) * 1000000000.0f)));
    }

    while (Application::mainThreadQueue.try_dequeue(job));

    userFunctionInvoker(EngineEvent::OnQuit);

    Entities::front->clear();
    Entities::front.reset();

    if (!Entities::table.empty()) [[unlikely]]
    {
        Debug::logError("Entities weren't fully cleaned up! This will cause problems!");
        Entities::table.clear();
    }

    CoreEngine::state[size_t(EngineState::MainThreadDone)].test_and_set();
    CoreEngine::state[size_t(EngineState::MainThreadDone)].notify_all();
}

void CoreEngine::internalWindowLoop()
{
    CoreEngine::state[size_t(EngineState::WindowInit)].wait(false, std::memory_order_relaxed);

    constexpr auto notifyEarlyFailure = []
    {
        CoreEngine::state[size_t(EngineState::ExitRequested)].test_and_set();
        CoreEngine::state[size_t(EngineState::ExitRequested)].notify_all();
        CoreEngine::state[size_t(EngineState::RenderInit)].test_and_set();
        CoreEngine::state[size_t(EngineState::RenderInit)].notify_all();
    };

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) [[unlikely]]
    {
        Debug::logError("Display initialisation failed! Error: ", SDL_GetError(), ".\n");
        notifyEarlyFailure();
        goto EarlyReturn;
    }

    if (!(CoreEngine::displMd = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay())))
    {
        Debug::logError("Failed to get desktop display details: ", SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        notifyEarlyFailure();
        goto EarlyReturn;
    }

    Screen::width = CoreEngine::displMd->w;
    Screen::height = CoreEngine::displMd->h;
    Screen::screenRefreshRate = CoreEngine::displMd->refresh_rate;

    CoreEngine::wind = SDL_CreateWindow(Window::_name.c_str(), +Window::width, +Window::height, SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!CoreEngine::wind)
    {
        Debug::logError("Could not create window: ", SDL_GetError(), ".");
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        notifyEarlyFailure();
        goto EarlyReturn;
    }

    SDL_SetWindowMinimumSize(wind, Window::_minimumSize.x, Window::_minimumSize.y);

    {
        func::function<void()> job;
        struct
        {
            int32_t w = 0, h = 0;
            std::atomic_flag shouldUnlock = ATOMIC_FLAG_INIT;
            const std::thread::id mustBe = std::this_thread::get_id();
            func::function<void()>& job;
        } resizeData { .job = job };

        bool (*eventWatcher)(void*, SDL_Event*) = [](void* data, SDL_Event* event) -> bool
        {
            decltype(resizeData)& windowSizeData = *_as(decltype(resizeData)*, data);

            while (Application::windowThreadQueue.try_dequeue(windowSizeData.job)) windowSizeData.job();

            if (windowSizeData.mustBe == std::this_thread::get_id() && windowSizeData.shouldUnlock.test())
            {
                windowSizeData.shouldUnlock.clear();
                CoreEngine::renderQueueLock.unlock();
            }
            if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
            {
                assert(windowSizeData.mustBe == std::this_thread::get_id());

                int32_t w = event->window.data1, h = event->window.data2;
                Application::queueJobForMainThread([w, h]
                {
                    Window::resizing = true;
                    i32 prevw = Window::width, prevh = Window::height;
                    Window::width = w;
                    Window::height = h;

                    userFunctionInvoker([&prevw, &prevh] { EngineEvent::OnWindowResize(glm::i32vec2 { prevw, prevh }); });
                });

                CoreEngine::renderQueueLock.lock();
                windowSizeData.shouldUnlock.test_and_set();
            }

            return 0;
        };
        SDL_AddEventWatch(eventWatcher, &resizeData);

        if (Window::_resizable)
            SDL_SetWindowResizable(CoreEngine::wind, true);

        CoreEngine::state[size_t(EngineState::RenderInit)].test_and_set(); // Spin off rendering thread.
        CoreEngine::state[size_t(EngineState::RenderInit)].notify_all();

        SDL_Event ev;
        while (!CoreEngine::state[size_t(EngineState::RenderThreadDone)].test())
        {
            while (Application::windowThreadQueue.try_dequeue(job)) job();

            if (SDL_PeepEvents(&ev, 1, SDL_PEEKEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST) && ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
            {
                assert(ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED);

                if (resizeData.shouldUnlock.test())
                {
                    resizeData.shouldUnlock.clear();
                    CoreEngine::renderQueueLock.unlock();
                }

                std::lock_guard guard(CoreEngine::renderQueueLock);
                SDL_PollEvent(&ev);
            }
            else if (SDL_PollEvent(&ev))
            {
                switch (ev.type)
                {
#pragma region Mouse Events
                case SDL_EVENT_MOUSE_MOTION:
                    Application::queueJobForMainThread([xPos = ev.motion.x, yPos = ev.motion.y, xMot = ev.motion.xrel, yMot = ev.motion.yrel]
                    {
                        glm::vec2 from = Input::internalMousePosition;
                        Input::internalMousePosition.x = xPos - Window::width / 2_u32;
                        Input::internalMousePosition.y = -yPos + Window::height / 2_u32;
                        Input::internalMouseMotion.x += xMot;
                        Input::internalMouseMotion.y -= yMot;
                        userFunctionInvoker([&from] { EngineEvent::OnMouseMove(from); });
                    });
                    break;
                case SDL_EVENT_MOUSE_WHEEL:
                    Application::queueJobForMainThread([xScr = ev.wheel.x, yScr = ev.wheel.y]
                    { userFunctionInvoker([&xScr, &yScr] { EngineEvent::OnMouseScroll(glm::vec2(xScr, yScr)); }); });
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    Application::queueJobForMainThread([button = Input::convertFromSDLMouse(ev.button.button)]
                    {
                        Input::heldMouseInputs[size_t(button)] = true;
                        userFunctionInvoker([&button] { EngineEvent::OnMouseDown(button); });
                    });
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    Application::queueJobForMainThread([button = Input::convertFromSDLMouse(ev.button.button)]
                    {
                        Input::heldMouseInputs[size_t(button)] = false;
                        userFunctionInvoker([&button] { EngineEvent::OnMouseUp(button); });
                    });
                    break;
#pragma endregion

#pragma region Key Events
                case SDL_EVENT_KEY_DOWN:
                    if (ev.key.repeat)
                        Application::queueJobForMainThread([key = Input::convertFromSDLKey(ev.key.key)] { userFunctionInvoker([&key] { EngineEvent::OnKeyRepeat(key); }); });
                    else
                    {
                        Application::queueJobForMainThread([key = Input::convertFromSDLKey(ev.key.key)]
                        {
                            Input::heldKeyInputs[size_t(key)] = true;
                            userFunctionInvoker([&key] { EngineEvent::OnKeyDown(key); });
                        });
                    }
                    break;
                case SDL_EVENT_KEY_UP:
                    Application::queueJobForMainThread([key = Input::convertFromSDLKey(ev.key.key)]
                    {
                        Input::heldKeyInputs[size_t(key)] = false;
                        userFunctionInvoker([&key] { EngineEvent::OnKeyUp(key); });
                    });
                    break;
#pragma endregion

                case SDL_EVENT_TEXT_INPUT:
                    {
                        std::u32string input;
                        for (auto it = ev.text.text; *it; ++it)
                        {
                            if ((*it & 0b11110000) == 0b11110000) // 4 bytes.
                            {
                                char32_t cAppend = (char32_t(*it & 0b00000111) << 18) | (char32_t(*(it + 1) & 0b00111111) << 12) | (char32_t(*(it + 2) & 0b00111111) << 6) |
                                    char32_t(*(it + 3) & 0b00111111);
                                input.push_back(cAppend);
                            }
                            else if ((*it & 0b11100000) == 0b11100000) // 3 bytes.
                            {
                                char32_t cAppend = char32_t(*it & 0b00001111) << 12 | char32_t(*(it + 1) & 0b00111111) << 6 | char32_t(*(it + 2) & 0b00111111);
                                input.push_back(cAppend);
                            }
                            else if ((*it & 0b11000000) == 0b11000000) // 2 bytes.
                            {
                                char32_t cAppend = char32_t(*it & 0b00011111) << 6 | char32_t(*(it + 1) & 0b00111111);
                                input.push_back(cAppend);
                            }
                            else
                                input.push_back(char32_t(*it)); // 1 byte.
                        }
                        Application::queueJobForMainThread([input = std::move(input)] { userFunctionInvoker([&input] { EngineEvent::OnTextInput(input); }); });
                    }
                    break;

                case SDL_EVENT_WINDOW_MOVED:
                    break;

                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                case SDL_EVENT_QUIT:
                    CoreEngine::state[size_t(EngineState::ExitRequested)].test_and_set();
                    CoreEngine::state[size_t(EngineState::ExitRequested)].notify_all();
                    break;
                }
            }
            else
                CoreEngine::waitSome();
        }

        SDL_DestroyWindow(CoreEngine::wind);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);

        SDL_RemoveEventWatch(eventWatcher, &resizeData);
    }
EarlyReturn:
    CoreEngine::state[size_t(EngineState::WindowThreadDone)].test_and_set(); // Signal main thread.
    CoreEngine::state[size_t(EngineState::WindowThreadDone)].notify_all();
}

void CoreEngine::internalRenderLoop()
{
    CoreEngine::state[size_t(EngineState::RenderInit)].wait(false, std::memory_order_relaxed);
    if (CoreEngine::state[size_t(EngineState::ExitRequested)].test()) // Window thread failed to initialize.
    {
        CoreEngine::state[size_t(EngineState::RenderThreadReady)].test_and_set();
        CoreEngine::state[size_t(EngineState::RenderThreadReady)].notify_all();
        goto EarlyReturn;
    }

    {
        RendererBackend initBackend = RendererBackend::Default;
        const RendererBackend backendPriorityOrder[] {
#if _WIN32
            RendererBackend::Vulkan, RendererBackend::Direct3D12, RendererBackend::Direct3D11, RendererBackend::OpenGL,
#else
            RendererBackend::Vulkan, RendererBackend::OpenGL,
#endif
            RendererBackend::Default
        };

        std::vector<RendererBackend> vecBackends = Renderer::platformBackends();
        std::set<RendererBackend> backends(vecBackends.begin(), vecBackends.end());
        for (RendererBackend targetBackend : backendPriorityOrder)
        {
            if (backends.contains(targetBackend))
            {
                initBackend = targetBackend;
                break;
            }
        }

        constexpr auto notifyEarlyFailure = []
        {
            CoreEngine::state[size_t(EngineState::ExitRequested)].test_and_set();
            CoreEngine::state[size_t(EngineState::ExitRequested)].notify_all();
            CoreEngine::state[size_t(EngineState::RenderThreadReady)].test_and_set();
            CoreEngine::state[size_t(EngineState::RenderThreadReady)].notify_all();
        };

        void *nwh = nullptr, *ndt = nullptr;
#if defined(SDL_PLATFORM_WIN32)
        nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(SDL_PLATFORM_MACOS)
        nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#elif defined(SDL_PLATFORM_LINUX)
        if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0)
        {
            ndt = SDL_GetPointerProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
            nwh = _asr(void*, SDL_GetNumberProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
        }
        else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0)
        {
            ndt = SDL_GetPointerProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
            nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
        }
        else
        {
            Debug::logError("Invalid Linux video driver!");
            notifyEarlyFailure();
            goto EarlyReturn;
        }

        if (!ndt)
        {
            Debug::logError("Failed to get Linux display!");
            notifyEarlyFailure();
            goto EarlyReturn;
        }
#elif defined(SDL_PLATFORM_IOS)
        nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr);
#endif

        if (!nwh)
        {
            Debug::logError("Failed to get window handle!");
            notifyEarlyFailure();
            goto EarlyReturn;
        }

        if (!RenderPipeline::renderInitialize(ndt, nwh, u32(Window::width), u32(Window::height), initBackend))
        {
            Debug::logError("Failed to initialize renderer!");
            notifyEarlyFailure();
            goto EarlyReturn;
        }
    }

    CoreEngine::state[size_t(EngineState::RenderThreadReady)].test_and_set(); // Signal main thread.
    CoreEngine::state[size_t(EngineState::RenderThreadReady)].notify_all();

    {
        RenderJob job;
        while (!CoreEngine::state[size_t(EngineState::MainThreadDone)].test())
        {
        WhileLoop:
            {
                std::lock_guard guard(CoreEngine::renderQueueLock);
                if (!CoreEngine::renderQueue.empty())
                {
                    job = std::move(CoreEngine::renderQueue.front());
                    CoreEngine::renderQueue.pop_front();
                    if (CoreEngine::renderQueue.size() >= Config::GraphicsQueueOverburdenedThreshold && !job.required())
                        Debug::logInfo("Render queue overburdened, skipping render job id ", static_cast<const void*>(&job.function()), ".\n");
                    else
                        job();
                    goto WhileLoop;
                }
            }

            CoreEngine::waitSome();
        }

        // Cleanup.
        while (!CoreEngine::renderQueue.empty())
        {
            job = std::move(CoreEngine::renderQueue.front());
            CoreEngine::renderQueue.pop_front();
            if (job.required())
                job();
        }
    }

    InternalEngineEvent::OnRenderShutdown();
    RenderPipeline::renderShutdown();
EarlyReturn:
    CoreEngine::state[size_t(EngineState::RenderThreadDone)].test_and_set(); // Signal window thread.
    CoreEngine::state[size_t(EngineState::RenderThreadDone)].notify_all();
}
