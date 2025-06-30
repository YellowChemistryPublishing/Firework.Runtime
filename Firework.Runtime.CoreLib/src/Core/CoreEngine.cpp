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
#include <Library/Hash.h>

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

    fs::path p = fs::current_path();
    p.append("RuntimeInternal");
    std::wstring dir = p.wstring();
    if (!fs::exists(dir))
        fs::create_directory(dir);

    uint16_t word = 0x0001;
    if (reinterpret_cast<uint8_t*>(&word)[0])
        PackageManager::endianness = Endianness::Little;
    else
        PackageManager::endianness = Endianness::Big;

    fs::path corePackagePath(fs::current_path());
    corePackagePath.append("Runtime");
    corePackagePath.append("CorePackage.fwpkg");
    if (fs::exists(corePackagePath))
    {
        Debug::logInfo("Loading CorePackage...");
        PackageManager::loadPackageIntoMemory(corePackagePath);
        Debug::logInfo("CorePackage loaded!");
    }
    else
        Debug::logError("The CorePackage could not be found in the Runtime folder. Did you accidentally delete it?");

    std::thread workerThread([]
    {
        func::function<void()> event;
        while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::WindowThreadDone)
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

    PackageManager::freePackageInMemory(corePackagePath);
    fs::remove_all(dir);

    SDL_Quit();

    return EXIT_SUCCESS;
}

void CoreEngine::resetDisplayData()
{
    if (!(CoreEngine::displMd = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay())))
        Debug::logError("Failed to get desktop display mode: ", SDL_GetError());
    Application::mainThreadQueue.enqueue([w = CoreEngine::displMd->w, h = CoreEngine::displMd->h, rr = CoreEngine::displMd->refresh_rate]
    {
        Screen::width = w;
        Screen::height = h;
        Screen::screenRefreshRate = rr;
    });
}

void CoreEngine::internalLoop()
{
    constexpr auto handled = []<typename Func>(Func&& func)
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

        // __hwTry
        // {
        func();
        // }
        // __hwCatch (const cpptrace::exception_with_message& ex)
        // {
        //     std::string traceback = "std::exception (";
        //     traceback
        //     .append(cpptrace::demangle(typeid(ex).name()))
        //     .append("): ")
        //     .append(ex.message())
        //     .append("\nUnhandled exception thrown, at:\n")
        //     .append(fmtTrace(ex.trace()));
        //     Debug::logError(traceback);
        // }
        // __hwCatch (const cpptrace::exception& ex)
        // {
        //     std::string traceback = "std::exception (";
        //     traceback
        //     .append(cpptrace::demangle(typeid(ex).name()))
        //     .append("): ")
        //     .append(ex.what())
        //     .append("\nUnhandled exception thrown, at:\n")
        //     .append(fmtTrace(ex.trace()));
        //     Debug::logError(traceback);
        // }
        // __hwCatch (const Exception& ex)
        // {
        //     std::string traceback = "std::exception (";
        //     traceback
        //     .append(cpptrace::demangle(typeid(ex).name()))
        //     .append("): ")
        //     .append(ex.what())
        //     .append("\nUnhandled exception thrown, at:\n")
        //     /*/.append(fmtTrace(ex.resolveStacktrace()))/*/;
        //     Debug::logError(traceback);
        // }
        // __hwCatch (const std::exception& ex)
        // {
        //     std::string traceback = "std::exception (";
        //     traceback
        //     .append(
        //     #if __has_include(<cpptrace/cpptrace.hpp>)
        //     cpptrace::demangle(
        //     #endif
        //     typeid(ex).name()
        //     #if __has_include(<cpptrace/cpptrace.hpp>)
        //     )
        //     #endif
        //     )
        //     .append("): ")
        //     .append(ex.what());
        //     #if __has_include(<cpptrace/cpptrace.hpp>)
        //     traceback
        //     .append("\nUnhandled exception thrown, at:\n")
        //     .append(fmtTrace(cpptrace::stacktrace::current()));
        //     #endif
        //     Debug::logError(traceback);
        // }
        // __hwCatch (...)
        // {
        //     #if defined(_GLIBCXX_RELEASE) && __has_include(<cpptrace/cpptrace.hpp>)
        //     std::string traceback = cpptrace::demangle(std::current_exception().__cxa_exception_type()->name());
        //     traceback.append(": ");
        //     #else
        //     std::string traceback;
        //     #endif
        //     traceback
        //     .append("[Unknown / JIT Compiled Code]");
        //     #if __has_include(<cpptrace/cpptrace.hpp>)
        //     traceback
        //     .append("\nUnhandled exception thrown, at:\n")
        //     .append(fmtTrace(cpptrace::stacktrace::current()));
        //     #endif
        //     Debug::logError(traceback);
        // }
        // __hwEnd();
    };

    CoreEngine::state.store(EngineState::WindowInit, std::memory_order_relaxed); // Spin off window handling thread.
    while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::RenderThreadReady)
    {
#ifdef __EMSCRIPTEN__
        emscripten_sleep(1);
#endif
    }
    CoreEngine::state.store(EngineState::Playing, std::memory_order_relaxed);

    EngineEvent::OnInitialize();

    func::function<void()> job;
    Uint64 frameBegin = SDL_GetPerformanceCounter();
    Uint64 perfFreq = SDL_GetPerformanceFrequency();
    float targetDeltaTime = Application::secondsPerFrame;
    float deltaTime = -1.0f;
    float prevw = +Window::width, prevh = +Window::height;

    EngineEvent::OnWindowResize(sysm::vector2i32 { Window::width, Window::height });

    while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::ExitRequested)
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
                    EngineEvent::OnMouseHeld((MouseButton)i);
            }
            for (uint_fast16_t i = 0; i < (uint_fast16_t)Key::Count; i++)
            {
                if (Input::heldKeyInputs[i])
                    EngineEvent::OnKeyHeld((Key)i);
            }
#pragma endregion

            handled([] { EngineEvent::OnTick(); });
            handled([] { EngineEvent::OnLateTick(); });

#pragma region Render Offload
            // My code is held together with glue and duct tape. And not the good stuff either.
            if (Window::width != prevw || Window::height != prevh)
            {
                CoreEngine::frameRenderJobs.push_back(RenderJob::create([w = Window::width, h = Window::height]
                {
                    RenderPipeline::resetBackbuffer(+u32(w), +u32(h));
                    RenderPipeline::resetViewArea(+u32(w), +u32(h));
                }));
                prevw = +Window::width;
                prevh = +Window::height;
            }
            CoreEngine::frameRenderJobs.push_back(RenderJob::create([] { RenderPipeline::clearViewArea(); }, false));

            sz renderIndex = 0;
            Entities::forEachEntity([&](Entity& entity)
            {
                for (auto& [typeIndex, componentSet] : Entities::table)
                {
                    auto componentIt = componentSet.find(&entity);
                    if (componentIt != componentSet.end())
                        InternalEngineEvent::OnRenderOffloadForComponent(typeIndex, entity, componentIt->second, renderIndex++);
                }
            });

            CoreEngine::frameRenderJobs.push_back(RenderJob::create([]
            {
                RenderPipeline::renderFrame();
                frameInProgress.clear(std::memory_order_relaxed);
            }, false));
            if (frameInProgress.test(std::memory_order_relaxed))
            {
                std::erase_if(CoreEngine::frameRenderJobs, [](const auto& job)
                {
                    if (!job.required)
                    {
                        job.destroy();
                        return true;
                    }
                    else
                        return false;
                });
            }
            else
                frameInProgress.test_and_set(std::memory_order_relaxed);
            if (!CoreEngine::frameRenderJobs.empty())
            {
                CoreEngine::renderQueue.enqueue(RenderJob::create([jobs = std::move(CoreEngine::frameRenderJobs)]
                {
                    renderResizeLock.lock();
                    for (auto job : jobs)
                    {
                        job();
                        job.destroy();
                    }
                    renderResizeLock.unlock();
                }));
                CoreEngine::frameRenderJobs.clear();
            }
#pragma endregion

#pragma region Post-Tick
            Input::internalMouseMotion = sysm::vector2::zero;

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
                std::chrono::nanoseconds((uint64_t)((targetDeltaTime - ((float)(SDL_GetPerformanceCounter() - frameBegin) / (float)perfFreq)) * 1000000000.0f)));
#endif
        }

#ifdef __EMSCRIPTEN__
        Debug::logTrace("Main loop end.");
        emscripten_sleep(std::max(unsigned(Application::secondsPerFrame * 1000.0f - 100.0f), 0u));
#endif
    }

    EngineEvent::OnQuit();

    while (Application::mainThreadQueue.try_dequeue(job));
    while (CoreEngine::pendingPreTickQueue.try_dequeue(job));
    while (CoreEngine::pendingPostTickQueue.try_dequeue(job));

    if (Entities::front)
        Entities::front.reset();

    // Render finalise.
    CoreEngine::renderQueue.enqueue(RenderJob::create([jobs = std::move(CoreEngine::frameRenderJobs)]
    {
        for (auto job : jobs)
        {
            if (job.required)
                job();
            job.destroy();
        }
    }));
    CoreEngine::frameRenderJobs.clear();

    CoreEngine::state.store(EngineState::MainThreadDone, std::memory_order_relaxed);
    // Wait for window handling thread to finish.
    while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::WindowThreadDone)
    {
#ifdef __EMSCRIPTEN__
        emscripten_sleep(1);
#endif
    }
}

void CoreEngine::internalWindowLoop()
{
    while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::WindowInit)
    {
#ifdef __EMSCRIPTEN__
        emscripten_sleep(1);
#endif
    }

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

    Window::width = Application::_initializationOptions.resolution.x;
    Window::height = Application::_initializationOptions.resolution.y;

    CoreEngine::wind = SDL_CreateWindow(Application::_initializationOptions.windowName.c_str(), +Application::_initializationOptions.resolution.x,
                                        +Application::_initializationOptions.resolution.y, SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!wind)
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

        bool (*eventWatcher)(void*, SDL_Event*) = [](void* _data, SDL_Event* event) -> bool
        // These few lines of code were literal years in the making. It formed part of my journey in learning C++.
        // I hope you like the rendering-while-resizing...
        {
            decltype(resizeData)& windowSizeData = *(decltype(resizeData)*)_data;

            while (Application::windowThreadQueue.try_dequeue(windowSizeData.job)) windowSizeData.job();

            if (std::lock_guard guard(windowSizeData.shouldUnlockLock); windowSizeData.shouldUnlock)
            {
                renderResizeLock.unlock();
                windowSizeData.shouldUnlock = false;
            }
            if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
            {
                windowSizeData.shouldUnlockLock.lock();
                windowSizeData.shouldUnlock = true;
                windowSizeData.shouldUnlockLock.unlock();

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

                    EngineEvent::OnWindowResize(sysm::vector2i32 { prevw, prevh });
                });

                renderResizeLock.lock();
            }

            return 0;
        };
        SDL_AddEventWatch(eventWatcher, &resizeData);

        if (Application::_initializationOptions.windowResizeable)
            SDL_SetWindowResizable(CoreEngine::wind, true);

        CoreEngine::state.store(EngineState::RenderInit, std::memory_order_relaxed); // Spin off rendering thread.

        SDL_Event ev;
        while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::RenderThreadDone)
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

                renderResizeLock.lock();
                SDL_PollEvent(&ev);
                renderResizeLock.unlock();
            }
            else if (SDL_PollEvent(&ev))
            {
                switch (ev.type)
                {
#pragma region Mouse Events
                case SDL_EVENT_MOUSE_MOTION:
                    Application::mainThreadQueue.enqueue([posX = ev.motion.x, posY = ev.motion.y, motX = ev.motion.xrel, motY = ev.motion.yrel]
                    {
                        sysm::vector2 from = Input::internalMousePosition;
                        Input::internalMousePosition.x = posX - Window::width / 2_u32;
                        Input::internalMousePosition.y = -posY + Window::height / 2_u32;
                        Input::internalMouseMotion.x += motX;
                        Input::internalMouseMotion.y -= motY;
                        EngineEvent::OnMouseMove(from);
                    });
                    break;
                case SDL_EVENT_MOUSE_WHEEL:
                    Application::mainThreadQueue.enqueue([scrX = ev.wheel.x, scrY = ev.wheel.y] { EngineEvent::OnMouseScroll(sysm::vector2(scrX, scrY)); });
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    Application::mainThreadQueue.enqueue([button = Input::convertFromSDLMouse(ev.button.button)]
                    {
                        Input::heldMouseInputs[(size_t)button] = true;
                        EngineEvent::OnMouseDown(button);
                    });
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    Application::mainThreadQueue.enqueue([button = Input::convertFromSDLMouse(ev.button.button)]
                    {
                        Input::heldMouseInputs[(size_t)button] = false;
                        EngineEvent::OnMouseUp(button);
                    });
                    break;
#pragma endregion
#pragma region Key Events
                case SDL_EVENT_KEY_DOWN:
                    switch (ev.key.repeat)
                    {
                    case true: Application::mainThreadQueue.enqueue([key = Input::convertFromSDLKey(ev.key.key)] { EngineEvent::OnKeyRepeat(key); }); break;
                    case false:
                        Application::mainThreadQueue.enqueue([key = Input::convertFromSDLKey(ev.key.key)]
                        {
                            Input::heldKeyInputs[(size_t)key] = true;
                            EngineEvent::OnKeyDown(key);
                        });
                        break;
                    }
                    break;
                case SDL_EVENT_KEY_UP:
                    Application::mainThreadQueue.enqueue([key = Input::convertFromSDLKey(ev.key.key)]
                    {
                        Input::heldKeyInputs[(size_t)key] = false;
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
                                char32_t cAppend = (char32_t)(c & 0b00000111) << 18 | (char32_t)(*(++it) & 0b00111111) << 12 | (char32_t)(*(++it) & 0b00111111) << 6 |
                                    (char32_t)(*(++it) & 0b00111111);
                                input.push_back(cAppend);
                            }
                            else if ((c & 0b11100000) == 0b11100000) // 3 bytes.
                            {
                                char32_t cAppend = (char32_t)(c & 0b00001111) << 12 | (char32_t)(*(++it) & 0b00111111) << 6 | (char32_t)(*(++it) & 0b00111111);
                                input.push_back(cAppend);
                            }
                            else if ((c & 0b11000000) == 0b11000000) // 2 bytes.
                            {
                                char32_t cAppend = (char32_t)(c & 0b00011111) << 6 | (char32_t)(*(++it) & 0b00111111);
                                input.push_back(cAppend);
                            }
                            else
                                input.push_back((char32_t)c); // 1 byte.
                        }
                        Application::mainThreadQueue.enqueue([input = std::move(input)] { EngineEvent::OnTextInput(input); });
                    }
                    break;
                case SDL_EVENT_WINDOW_MOVED: break;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                case SDL_EVENT_QUIT: CoreEngine::state.store(EngineState::ExitRequested, std::memory_order_relaxed); break;
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

#ifdef __EMSCRIPTEN__
            Debug::logTrace("Event loop end.");
            emscripten_sleep(1);
#endif
        }

        SDL_DestroyWindow(CoreEngine::wind);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);

        SDL_RemoveEventWatch(eventWatcher, &resizeData);
    }

EarlyReturn:
    CoreEngine::state.store(EngineState::WindowThreadDone, std::memory_order_relaxed); // Signal main thread.
}

void CoreEngine::internalRenderLoop()
{
    while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::RenderInit)
    {
#ifdef __EMSCRIPTEN__
        emscripten_sleep(1);
#endif
    }

    RendererBackend initBackend = RendererBackend::Default;
    RendererBackend backendPriorityOrder[] {
#if _WIN32
        RendererBackend::OpenGL, RendererBackend::Vulkan, RendererBackend::Direct3D12, RendererBackend::Direct3D11
#else
        RendererBackend::Vulkan, RendererBackend::OpenGL
#endif
    };
    std::vector<RendererBackend> backends = Renderer::platformBackends();
    for (auto targetBackend : std::span(backendPriorityOrder, sizeof(backendPriorityOrder) / sizeof(*backendPriorityOrder)))
    {
        for (auto platformBackend : backends)
        {
            if (targetBackend == platformBackend)
            {
                initBackend = targetBackend;
                goto BreakAll;
            }
        }
    }
BreakAll:;

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

    if (!Renderer::initialize(ndt, nwh, +u32(Window::width), +u32(Window::height), initBackend))
    {
        Debug::logError("Failed to initialize renderer!");
        goto EarlyReturn;
    }

    RenderPipeline::resetViewArea(+u16(Window::width), +u16(Window::height));
    RenderPipeline::clearViewArea();

    CoreEngine::state.store(EngineState::RenderThreadReady, std::memory_order_relaxed); // Signal main thread.

    {
        RenderJob job;
        while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::MainThreadDone)
        {
            while (renderQueue.try_dequeue(job))
            {
                if (renderQueue.size_approx() > GL_QUEUE_OVERBURDENED_THRESHOLD && !job.required)
                    Debug::logInfo("Render queue overburdened, skipping render job id ", job.callFunction, ".\n");
                else
                    job();
                job.destroy();
            }

#if FIREWORK_LATENCY_TRADE == FIREWORK_LATENCY_TRADE_THREAD_YIELD
            std::this_thread::yield();
#elif FIREWORK_LATENCY_TRADE == FIREWORK_LATENCY_TRADE_THREAD_SLEEP
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif

#ifdef __EMSCRIPTEN__
            Debug::logTrace("Render loop end.");
            emscripten_sleep(1);
#endif
        }

        // Cleanup.
        while (renderQueue.try_dequeue(job))
        {
            if (job.required)
                job();
            job.destroy();
        }
    }

    InternalEngineEvent::OnRenderShutdown();
    if (!(FIREWORK_BUILD_PLATFORM == FIREWORK_BUILD_PLATFORM_LINUX && initBackend == RendererBackend::OpenGL))
        Renderer::shutdown();

EarlyReturn:
    CoreEngine::state.store(EngineState::RenderThreadDone, std::memory_order_relaxed); // Signal window thread.
}
