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
#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/threading.h>
#endif
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <numeric>
#if __linux__
#include <pthread.h>
#endif
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

std::atomic<EngineState> CoreEngine::state(EngineState::FirstInit);

SDL_Window* CoreEngine::wind = nullptr;
SDL_Renderer* CoreEngine::rend = nullptr;
const SDL_DisplayMode* CoreEngine::displMd;

moodycamel::ConcurrentQueue<func::function<void()>> CoreEngine::pendingPreTickQueue;
moodycamel::ConcurrentQueue<func::function<void()>> CoreEngine::pendingPostTickQueue;

moodycamel::ConcurrentQueue<RenderJob> CoreEngine::renderQueue;
std::vector<RenderJob> CoreEngine::frameRenderJobs;
std::atomic_flag frameInProgress = ATOMIC_FLAG_INIT;
std::mutex renderResizeLock;

int CoreEngine::execute(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

#if _WIN32
    if (IsWindowsVistaOrGreater())
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
        SetConsoleMode(hOut, dwMode);
    }
#endif

    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_HAPTIC
#if !__EMSCRIPTEN__
                  | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK
#endif
                  ))
    {
        Debug::logError("Runtime failed to initialize! Error: ", SDL_GetError(), '.');
        return EXIT_FAILURE;
    }

    std::thread workerThread([]
    {
        func::function<void()> event;
        while (CoreEngine::state.load(std::memory_order_relaxed) < EngineState::WindowThreadDone)
        {
            while (Application::workerThreadQueue.try_dequeue(event)) event();

#if FIREWORK_LATENCY_TRADE == FIREWORK_LATENCY_TRADE_THREAD_YIELD
            std::this_thread::yield();
#elif FIREWORK_LATENCY_TRADE == FIREWORK_LATENCY_TRADE_THREAD_SLEEP
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
        }
        while (Application::workerThreadQueue.try_dequeue(event)) event();
    });

    std::thread windowThread(internalWindowLoop);

    std::thread mainThread(internalLoop);

#ifdef __EMSCRIPTEN__
    internalRenderLoop();
#else
    std::thread renderThread(internalRenderLoop);
#endif

    mainThread.join();
#ifndef __EMSCRIPTEN__
    renderThread.join();
#endif
    windowThread.join();
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

    Application::mainThreadQueue.enqueue([w = CoreEngine::displMd->w, h = CoreEngine::displMd->h, rr = CoreEngine::displMd->refresh_rate]
    {
        Screen::width = w;
        Screen::height = h;
        Screen::screenRefreshRate = rr;
    });
}

constexpr static auto userFunctionInvoker = []<typename Func>(Func&& func)
{
#if __has_include(<cpptrace/cpptrace.hpp>)
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
#endif

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
        traceback.append(cpptrace::demangle(typeid(ex).name())).append("): ").append(ex.what()).append("\nUnhandled exception thrown, at:\n")
            /*/.append(fmtTrace(ex.resolveStacktrace()))/*/;
        Debug::logError(traceback);
    }
    __hwCatch(const std::exception& ex)
    {
        std::string traceback = "std::exception (";
        traceback
            .append(
#if __has_include(<cpptrace/cpptrace.hpp>)
                cpptrace::demangle(
#endif
                    typeid(ex).name()
#if __has_include(<cpptrace/cpptrace.hpp>)
                        )
#endif
                    )
            .append("): ")
            .append(ex.what());
#if __has_include(<cpptrace/cpptrace.hpp>)
        traceback.append("\nUnhandled exception thrown, at:\n").append(fmtTrace(cpptrace::stacktrace::current()));
#endif
        Debug::logError(traceback);
    }
    __hwCatch(...)
    {
#if defined(_GLIBCXX_RELEASE) && __has_include(<cpptrace/cpptrace.hpp>)
        std::string traceback = cpptrace::demangle(std::current_exception().__cxa_exception_type()->name());
        traceback.append(": ");
#else
        std::string traceback;
#endif
        traceback.append("[Unknown / JIT Compiled Code]");
#if __has_include(<cpptrace/cpptrace.hpp>)
        traceback.append("\nUnhandled exception thrown, at:\n").append(fmtTrace(cpptrace::stacktrace::current()));
#endif
        Debug::logError(traceback);
    }
    __hwEnd();
};

void CoreEngine::internalLoop()
{
    CoreEngine::state.store(EngineState::WindowInit, std::memory_order_seq_cst); // Spin off window handling thread.
    while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::RenderThreadReady) std::this_thread::yield();
    CoreEngine::state.store(EngineState::Playing, std::memory_order_seq_cst);

    userFunctionInvoker(EngineEvent::OnInitialize);

    func::function<void()> job;
    u64 frameBegin = SDL_GetPerformanceCounter();
    u64 perfFreq = SDL_GetPerformanceFrequency();
    float targetDeltaTime = Application::secondsPerFrame;
    float deltaTime = -1.0f;
    float prevw = float(+Window::width), prevh = float(+Window::height);

    userFunctionInvoker([&] { EngineEvent::OnWindowResize(glm::i32vec2 { Window::width, Window::height }); });

    while (CoreEngine::state.load(std::memory_order_relaxed) < EngineState::ExitRequested)
    {
        deltaTime = (float)(SDL_GetPerformanceCounter() - frameBegin) / (float)perfFreq;
        if (Application::mainThreadQueue.try_dequeue(job))
            job();
        else if (deltaTime >= targetDeltaTime)
        {
#pragma region Pre-Tick
            while (CoreEngine::pendingPreTickQueue.try_dequeue(job)) job();

            for (uint_fast16_t i = 0; i < (uint_fast16_t)MouseButton::Count; i++)
            {
                if (Input::heldMouseInputs[i])
                    userFunctionInvoker([&] { EngineEvent::OnMouseHeld(MouseButton(i)); });
            }
            for (uint_fast16_t i = 0; i < (uint_fast16_t)Key::Count; i++)
            {
                if (Input::heldKeyInputs[i])
                    userFunctionInvoker([&] { EngineEvent::OnKeyHeld(Key(i)); });
            }
#pragma endregion

            userFunctionInvoker(EngineEvent::OnTick);
            userFunctionInvoker(EngineEvent::OnLateTick);

#pragma region Render Offload
            // My code is held together with glue and duct tape. And not the good stuff either.
            if (Window::width != prevw || Window::height != prevh)
            {
                CoreEngine::frameRenderJobs.push_back(RenderJob([w = Window::width, h = Window::height]
                {
                    RenderPipeline::resetBackbuffer(+u32(w), +u32(h));
                    RenderPipeline::resetViewArea(+u16(w), +u16(h));
                }));
                prevw = float(+Window::width);
                prevh = float(+Window::height);
            }
            CoreEngine::frameRenderJobs.push_back(RenderJob([] { RenderPipeline::clearViewArea(); }, false));

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

            CoreEngine::frameRenderJobs.push_back(RenderJob([]
            {
                RenderPipeline::renderFrame();
                frameInProgress.clear(std::memory_order_relaxed);
            }, false));

            if (frameInProgress.test(std::memory_order_relaxed))
            {
                std::erase_if(CoreEngine::frameRenderJobs, [](const RenderJob& job) { return !job.required(); });
            }
            else
                frameInProgress.test_and_set(std::memory_order_relaxed);

            if (!CoreEngine::frameRenderJobs.empty())
            {
                CoreEngine::renderQueue.enqueue(RenderJob([jobs = std::move(CoreEngine::frameRenderJobs)]
                {
                    renderResizeLock.lock();
                    for (const RenderJob& job : jobs) job();
                    renderResizeLock.unlock();
                }));
                CoreEngine::frameRenderJobs.clear();
            }
#pragma endregion

#pragma region Post-Tick
            Input::internalMouseMotion = glm::vec2(0.0f);

            while (CoreEngine::pendingPostTickQueue.try_dequeue(job)) job();
#pragma endregion

            frameBegin = SDL_GetPerformanceCounter();
            targetDeltaTime = std::max(Application::secondsPerFrame - (deltaTime - targetDeltaTime), 0.0f);
            Time::frameDeltaTime = deltaTime * Time::timeScale;
        }
        else
        {
#if FIREWORK_LATENCY_TRADE == FIREWORK_LATENCY_TRADE_THREAD_YIELD
            std::this_thread::yield();
#elif FIREWORK_LATENCY_TRADE == FIREWORK_LATENCY_TRADE_THREAD_SLEEP
            std::this_thread::sleep_for(
                std::chrono::nanoseconds(uint64_t((targetDeltaTime - (float(SDL_GetPerformanceCounter() - frameBegin) / float(perfFreq))) * 1000000000.0f)));
#endif
        }
    }

    EngineEvent::OnQuit();

    while (Application::mainThreadQueue.try_dequeue(job));
    while (CoreEngine::pendingPreTickQueue.try_dequeue(job));
    while (CoreEngine::pendingPostTickQueue.try_dequeue(job));

    Entities::front->clear();
    Entities::front.reset();

    // Render finalise.
    CoreEngine::renderQueue.enqueue(RenderJob([jobs = std::move(CoreEngine::frameRenderJobs)]
    {
        for (const RenderJob& job : jobs)
        {
            if (job.required())
                job();
        }
    }));
    CoreEngine::frameRenderJobs.clear();

    CoreEngine::state.store(EngineState::MainThreadDone, std::memory_order_seq_cst);
}

void CoreEngine::internalWindowLoop()
{
    while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::WindowInit) std::this_thread::yield();

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) [[unlikely]]
    {
        Debug::logError("Display initialisation failed! Error: ", SDL_GetError(), ".\n");
        goto EarlyReturn;
    }

    if (!(CoreEngine::displMd = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay())))
    {
        Debug::logError("Failed to get desktop display details: ", SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        goto EarlyReturn;
    }

    Screen::width = CoreEngine::displMd->w;
    Screen::height = CoreEngine::displMd->h;
    Screen::screenRefreshRate = CoreEngine::displMd->refresh_rate;

    CoreEngine::wind = SDL_CreateWindow(Application::_initializationOptions.windowName.c_str(), +Window::width, +Window::height, SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!CoreEngine::wind)
    {
        Debug::logError("Could not create window: ", SDL_GetError(), ".");
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        goto EarlyReturn;
    }

    SDL_SetWindowMinimumSize(wind, 200, 200);

    {
        func::function<void()> job;
        struct
        {
            int32_t w, h;
            bool shouldUnlock = false;
            std::mutex shouldUnlockLock;
            func::function<void()>& job;
        } resizeData { .job = job };

        auto eventWatcher = +[](void* data, SDL_Event* event) -> bool
        {
            decltype(resizeData)& windowSizeData = *_as(decltype(resizeData)*, data);

            while (Application::windowThreadQueue.try_dequeue(windowSizeData.job)) windowSizeData.job();

            if (std::lock_guard guard(windowSizeData.shouldUnlockLock); windowSizeData.shouldUnlock)
            {
                renderResizeLock.unlock();
                windowSizeData.shouldUnlock = false;
            }
            if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
            {
                {
                    std::lock_guard guard(windowSizeData.shouldUnlockLock);
                    windowSizeData.shouldUnlock = true;
                }

                int32_t w = event->window.data1, h = event->window.data2;
                Application::mainThreadQueue.enqueue([w, h]
                {
                    Window::resizing = true;
                    i32 prevw = Window::width, prevh = Window::height;
                    Window::width = w;
                    Window::height = h;

                    RectFloat windowDelta = RectFloat {
                        (float)(h - prevh) / 2.0f,
                        (float)(w - prevw) / 2.0f,
                        (float)(-h + prevh) / 2.0f,
                        (float)(-w + prevw) / 2.0f,
                    };

                    userFunctionInvoker([&] { EngineEvent::OnWindowResize(glm::i32vec2 { prevw, prevh }); });
                });

                renderResizeLock.lock();
            }

            return 0;
        };
        SDL_AddEventWatch(eventWatcher, &resizeData);

        if (Application::_initializationOptions.windowResizeable)
            SDL_SetWindowResizable(CoreEngine::wind, true);

        CoreEngine::state.store(EngineState::RenderInit, std::memory_order_seq_cst); // Spin off rendering thread.

        SDL_Event ev;
        while (CoreEngine::state.load(std::memory_order_relaxed) < EngineState::RenderThreadDone)
        {
            while (Application::windowThreadQueue.try_dequeue(job)) job();

            if (SDL_PeepEvents(&ev, 1, SDL_PEEKEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST) && ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
            {
                assert(ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED);

                if (std::lock_guard guard(resizeData.shouldUnlockLock); resizeData.shouldUnlock)
                {
                    renderResizeLock.unlock();
                    resizeData.shouldUnlock = false;
                }

                std::lock_guard guard(renderResizeLock);
                SDL_PollEvent(&ev);
            }
            else if (SDL_PollEvent(&ev))
            {
                switch (ev.type)
                {
#pragma region Mouse Events
                case SDL_EVENT_MOUSE_MOTION:
                    Application::mainThreadQueue.enqueue([posX = ev.motion.x, posY = ev.motion.y, motX = ev.motion.xrel, motY = ev.motion.yrel]
                    {
                        glm::vec2 from = Input::internalMousePosition;
                        Input::internalMousePosition.x = posX - Window::width / 2_u32;
                        Input::internalMousePosition.y = -posY + Window::height / 2_u32;
                        Input::internalMouseMotion.x += motX;
                        Input::internalMouseMotion.y -= motY;
                        userFunctionInvoker([&] { EngineEvent::OnMouseMove(from); });
                    });
                    break;
                case SDL_EVENT_MOUSE_WHEEL:
                    Application::mainThreadQueue.enqueue([scrX = ev.wheel.x, scrY = ev.wheel.y]
                    { userFunctionInvoker([&] { EngineEvent::OnMouseScroll(glm::vec2(scrX, scrY)); }); });
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    Application::mainThreadQueue.enqueue([button = Input::convertFromSDLMouse(ev.button.button)]
                    {
                        Input::heldMouseInputs[size_t(button)] = true;
                        userFunctionInvoker([&] { EngineEvent::OnMouseDown(button); });
                    });
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    Application::mainThreadQueue.enqueue([button = Input::convertFromSDLMouse(ev.button.button)]
                    {
                        Input::heldMouseInputs[size_t(button)] = false;
                        userFunctionInvoker([&] { EngineEvent::OnMouseUp(button); });
                    });
                    break;
#pragma endregion
#pragma region Key Events
                case SDL_EVENT_KEY_DOWN:
                    if (ev.key.repeat)
                        Application::mainThreadQueue.enqueue([key = Input::convertFromSDLKey(ev.key.key)] { EngineEvent::OnKeyRepeat(key); });
                    else
                    {
                        Application::mainThreadQueue.enqueue([key = Input::convertFromSDLKey(ev.key.key)]
                        {
                            Input::heldKeyInputs[size_t(key)] = true;
                            EngineEvent::OnKeyDown(key);
                        });
                    }
                    break;
                case SDL_EVENT_KEY_UP:
                    Application::mainThreadQueue.enqueue([key = Input::convertFromSDLKey(ev.key.key)]
                    {
                        Input::heldKeyInputs[size_t(key)] = false;
                        EngineEvent::OnKeyUp(key);
                    });
                    break;
#pragma endregion
                case SDL_EVENT_TEXT_INPUT:
                    {
                        std::u32string input;
                        for (auto it = ev.text.text; *it; ++it)
                        {
                            char8_t c = *it;
                            if ((c & 0b11110000) == 0b11110000) // 4 bytes.
                            {
                                char32_t cAppend =
                                    char32_t(c & 0b00000111) << 18 | char32_t(*(++it) & 0b00111111) << 12 | char32_t(*(++it) & 0b00111111) << 6 | char32_t(*(++it) & 0b00111111);
                                input.push_back(cAppend);
                            }
                            else if ((c & 0b11100000) == 0b11100000) // 3 bytes.
                            {
                                char32_t cAppend = char32_t(c & 0b00001111) << 12 | char32_t(*(++it) & 0b00111111) << 6 | char32_t(*(++it) & 0b00111111);
                                input.push_back(cAppend);
                            }
                            else if ((c & 0b11000000) == 0b11000000) // 2 bytes.
                            {
                                char32_t cAppend = char32_t(c & 0b00011111) << 6 | char32_t(*(++it) & 0b00111111);
                                input.push_back(cAppend);
                            }
                            else
                                input.push_back(char32_t(c)); // 1 byte.
                        }
                        Application::mainThreadQueue.enqueue([input = std::move(input)] { userFunctionInvoker([&] { EngineEvent::OnTextInput(input); }); });
                    }
                    break;
                case SDL_EVENT_WINDOW_MOVED:
                    break;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                case SDL_EVENT_QUIT:
                    CoreEngine::state.store(EngineState::ExitRequested, std::memory_order_seq_cst);
                    break;
                }
            }
            else
            {
#if FIREWORK_LATENCY_TRADE == FIREWORK_LATENCY_TRADE_THREAD_YIELD
                std::this_thread::yield();
#elif FIREWORK_LATENCY_TRADE == FIREWORK_LATENCY_TRADE_THREAD_SLEEP
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
            }
        }

        SDL_DestroyWindow(CoreEngine::wind);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);

        SDL_RemoveEventWatch(eventWatcher, &resizeData);
    }
EarlyReturn:
    CoreEngine::state.store(EngineState::WindowThreadDone, std::memory_order_seq_cst); // Signal main thread.
}

void CoreEngine::internalRenderLoop()
{
    while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::RenderInit) std::this_thread::yield();

    RendererBackend initBackend = RendererBackend::Default;
    RendererBackend backendPriorityOrder[] {
#if _WIN32
        RendererBackend::OpenGL, RendererBackend::Direct3D12, RendererBackend::Vulkan, RendererBackend::Direct3D11,
#else
        RendererBackend::Vulkan, RendererBackend::OpenGL,
#endif
        RendererBackend::Default
    };

    std::vector<RendererBackend> backends = Renderer::platformBackends();
    for (RendererBackend targetBackend : std::span(backendPriorityOrder, sizeof(backendPriorityOrder) / sizeof(*backendPriorityOrder)))
    {
        for (RendererBackend platformBackend : backends)
        {
            if (targetBackend == platformBackend)
            {
                initBackend = targetBackend;
                goto BreakAll;
            }
        }
    }
BreakAll:
    void *nwh = nullptr, *ndt = nullptr;
#if defined(SDL_PLATFORM_WIN32)
    nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#elif defined(SDL_PLATFORM_MACOS)
    nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
#elif defined(SDL_PLATFORM_LINUX)
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0)
    {
        ndt = SDL_GetPointerProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        nwh = (void*)SDL_GetNumberProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
    }
    else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0)
    {
        ndt = SDL_GetPointerProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
        nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
    }
    else
    {
        Debug::logError("Invalid Linux video driver!");
        goto EarlyReturn;
    }

    if (!ndt)
    {
        Debug::logError("Failed to get Linux display!");
        goto EarlyReturn;
    }
#elif defined(SDL_PLATFORM_IOS)
    nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(CoreEngine::wind), SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
#endif

    if (!nwh)
    {
        Debug::logError("Failed to get window handle!");
        goto EarlyReturn;
    }

    if (!RenderPipeline::renderInitialize(ndt, nwh, u32(Window::width), u32(Window::height), initBackend))
    {
        Debug::logError("Failed to initialize renderer!");
        goto EarlyReturn;
    }

    CoreEngine::state.store(EngineState::RenderThreadReady, std::memory_order_seq_cst); // Signal main thread.

    {
        RenderJob job;
        while (CoreEngine::state.load(std::memory_order_relaxed) < EngineState::MainThreadDone)
        {
            while (renderQueue.try_dequeue(job))
            {
                if (renderQueue.size_approx() > GL_QUEUE_OVERBURDENED_THRESHOLD && !job.required())
                    Debug::logInfo("Render queue overburdened, skipping render job id ", static_cast<const void*>(&job.function()), ".\n");
                else
                    job();
            }

#if FIREWORK_LATENCY_TRADE == FIREWORK_LATENCY_TRADE_THREAD_YIELD
            std::this_thread::yield();
#elif FIREWORK_LATENCY_TRADE == FIREWORK_LATENCY_TRADE_THREAD_SLEEP
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
        }

        // Cleanup.
        while (renderQueue.try_dequeue(job))
        {
            if (job.required())
                job();
        }
    }

    InternalEngineEvent::OnRenderShutdown();
    RenderPipeline::renderShutdown();
EarlyReturn:
    CoreEngine::state.store(EngineState::RenderThreadDone, std::memory_order_seq_cst); // Signal window thread.
}
