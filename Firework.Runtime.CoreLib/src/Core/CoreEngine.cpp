#include "CoreEngine.h"

#include <algorithm>
#include <cmath>
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
#include <SDL_video.h>
#include <SDL_pixels.h>
#include <span>
#if _WIN32
#include <VersionHelpers.h>
#endif

#include <Mathematics.h>
#include <Core/Application.h>
#include <Core/Debug.h>
#include <Core/Display.h>
#include <Core/HardwareExcept.h>
#include <Core/Input.h>
#include <Core/PackageManager.h>
#include <Core/Time.h>
#include <EntityComponentSystem/SceneManagement.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <Firework/Config.h>
#include <GL/Renderer.h>
#include <Library/Hash.h>
#include <Objects/Entity2D.h>

namespace fs = std::filesystem;
using namespace Firework;
using namespace Firework::GL;
using namespace Firework::Internal;
using namespace Firework::Mathematics;
using namespace Firework::PackageSystem;

std::atomic<EngineState> CoreEngine::state(EngineState::FirstInit);

SDL_Window* CoreEngine::wind = nullptr;
SDL_Renderer* CoreEngine::rend = nullptr;
const SDL_DisplayMode* CoreEngine::displMd;
SDL_SysWMinfo CoreEngine::wmInfo;
SDL_version CoreEngine::backendVer;

moodycamel::ConcurrentQueue<func::function<void()>> CoreEngine::pendingPreTickQueue;
moodycamel::ConcurrentQueue<func::function<void()>> CoreEngine::pendingPostTickQueue;

moodycamel::ConcurrentQueue<RenderJob> CoreEngine::renderQueue;
std::vector<RenderJob> CoreEngine::frameRenderJobs;
std::atomic_flag frameInProgress = ATOMIC_FLAG_INIT;
Firework::SpinLock renderResizeLock;

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

    if (!(SDL_Init(
        SDL_INIT_EVENTS |
        SDL_INIT_HAPTIC
        #if !__EMSCRIPTEN__
        | SDL_INIT_GAMEPAD
        | SDL_INIT_JOYSTICK
        #endif
        ) == 0))
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
    if (((uint8_t*)&word)[0])
        PackageManager::endianness = Endianness::Little;
    else PackageManager::endianness = Endianness::Big;
        
    fs::path corePackagePath(fs::current_path());
    corePackagePath.append("Runtime");
    corePackagePath.append("CorePackage.fwpkg");
    if (fs::exists(corePackagePath))
    {
        Debug::logInfo("Loading CorePackage...");
        PackageManager::loadCorePackageIntoMemory(corePackagePath);
        Debug::logInfo("CorePackage loaded!");
    }
    else Debug::logError("The CorePackage could not be found in the Runtime folder. Did you accidentally delete it?");

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

    // Cleanup here is done for stuff created in CoreEngine::execute, thread-specific cleanup is done per-thread, at the end of their lifetime.

    PackageManager::freeCorePackageInMemory();
    fs::remove_all(dir);

    SDL_Quit();

    return EXIT_SUCCESS;
}

void CoreEngine::resetDisplayData()
{
    if (!(CoreEngine::displMd = SDL_GetDesktopDisplayMode(1)))
        Debug::logError("Failed to get desktop display mode: ", SDL_GetError());
    Application::mainThreadQueue.enqueue([w = CoreEngine::displMd->w, h = CoreEngine::displMd->h, rr = CoreEngine::displMd->refresh_rate]
    {
        Screen::width = w;
        Screen::height = h;
        Screen::screenRefreshRate = rr;
    });
}

void __fw_rt_hw_eh_frame(auto&& func)
{
    __hwTry
    {
        func();
    }
    __hwExcept();
}
void CoreEngine::internalLoop()
{
    constexpr auto handled = []<typename Func>(Func&& func)
    {
        #if __has_include(<cpptrace/cpptrace.hpp>)
        constexpr auto fmtTrace = [](cpptrace::stacktrace ret) -> std::string
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

        try
        {
            __fw_rt_hw_eh_frame(std::move(func));
        }
        #if __has_include(<cpptrace/cpptrace.hpp>)
        catch (const cpptrace::exception_with_message& ex)
        {
            std::string traceback = "std::exception (";
            traceback
            .append(cpptrace::demangle(typeid(ex).name()))
            .append("): ")
            .append(ex.get_message())
            .append("\nUnhandled exception thrown, at:\n")
            .append(fmtTrace(ex.get_raw_trace().resolve()));
            Debug::logError(traceback);
        }
        catch (const cpptrace::exception& ex)
        {
            std::string traceback = "std::exception (";
            traceback
            .append(cpptrace::demangle(typeid(ex).name()))
            .append("): ")
            .append(ex.what())
            .append("\nUnhandled exception thrown, at:\n")
            .append(fmtTrace(ex.get_raw_trace().resolve()));
            Debug::logError(traceback);
        }
        catch (const Exception& ex)
        {
            std::string traceback = "std::exception (";
            traceback
            .append(cpptrace::demangle(typeid(ex).name()))
            .append("): ")
            .append(ex.what())
            .append("\nUnhandled exception thrown, at:\n")
            .append(fmtTrace(ex.trace));
            Debug::logError(traceback);
        }
        #endif
        catch (const std::exception& ex)
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
            traceback
            .append("\nUnhandled exception thrown, at:\n")
            .append(fmtTrace(cpptrace::stacktrace::current()));
            #endif
            Debug::logError(traceback);
        }
        catch(...)
        {
            #if defined(_GLIBCXX_RELEASE) && __has_include(<cpptrace/cpptrace.hpp>)
            std::string traceback = cpptrace::demangle(std::current_exception().__cxa_exception_type()->name());
            traceback.append(": ");
            #else
            std::string traceback;
            #endif
            traceback
            .append("[Unknown / JIT Compiled Code]");
            #if __has_include(<cpptrace/cpptrace.hpp>)
            traceback
            .append("\nUnhandled exception thrown, at:\n")
            .append(fmtTrace(cpptrace::stacktrace::current()));
            #endif
            Debug::logError(traceback);
        }
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

    func::function<void()> event;
    Uint64 frameBegin = SDL_GetPerformanceCounter();
    Uint64 perfFreq = SDL_GetPerformanceFrequency();
    float targetDeltaTime = Application::secondsPerFrame;
    float deltaTime = -1.0f;
    float prevw = Window::width, prevh = Window::height;

    while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::ExitRequested)
    {
        deltaTime = (float)(SDL_GetPerformanceCounter() - frameBegin) / (float)perfFreq;
        if (Application::mainThreadQueue.try_dequeue(event))
            event();
        else if (deltaTime >= targetDeltaTime)
        {
            #pragma region Pre-Tick
            while (CoreEngine::pendingPreTickQueue.try_dequeue(event))
                event();
                
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

            #pragma region Post-Tick
            Input::internalMouseMotion = Vector2Int(0, 0);

            while (CoreEngine::pendingPostTickQueue.try_dequeue(event))
                event();
            #pragma endregion

            #pragma region Render Offload
            // My code is held together with glue and duct tape. And not the good stuff either.
            if (Window::width != prevw || Window::height != prevh)
            {
                CoreEngine::frameRenderJobs.push_back(RenderJob::create([w = Window::width, h = Window::height]
                {
                    Renderer::resetBackbuffer((uint32_t)w, (uint32_t)h);
                    Renderer::setViewArea(0, 0, 0, (uint16_t)w, (uint16_t)h);
                    Renderer::setViewArea(1, 0, 0, (uint16_t)w, (uint16_t)h);
                    Renderer::setViewOrthographic(1, w, h, Vector3(0, 0, 0), Renderer::fromEuler(Vector3(0, 0, 0)), -16777216.0f, 16777216.0f);
                }));
                prevw = Window::width; prevh = Window::height;
            }
            CoreEngine::frameRenderJobs.push_back(RenderJob::create([]
            {
                Renderer::setViewClear(0, 0x000000ff, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
            }, false));

            for (auto _it1 = SceneManager::existingScenes.rbegin(); _it1 != SceneManager::existingScenes.rend(); ++_it1)
            {
                Scene* it1 = reinterpret_cast<Scene*>(&_it1->data);
                if (it1->active)
                {
                    for (auto it2 = it1->back; it2 != nullptr; it2 = it2->prev)
                    {
                        for (auto it3 = EntityManager::existingComponents.begin(); it3 != EntityManager::existingComponents.end(); ++it3)
                        {
                            auto component = EntityManager::components.find(std::make_pair(it2, it3->first));
                            if (component != EntityManager::components.end() && component->second->active)
                                InternalEngineEvent::OnRenderOffloadForComponent(component->second);
                        }
                    }
                    for (auto it2 = it1->back2D; it2 != nullptr; it2 = it2->prev)
                    {
                        for (auto it3 = EntityManager2D::existingComponents.begin(); it3 != EntityManager2D::existingComponents.end(); ++it3)
                        {
                            auto component = EntityManager2D::components.find(std::make_pair(it2, it3->first));
                            if (component != EntityManager2D::components.end() && component->second->active)
                                InternalEngineEvent::OnRenderOffloadForComponent2D(component->second);
                        }
                    }
                }
            }
            
            CoreEngine::frameRenderJobs.push_back(RenderJob::create([]
            {
                Renderer::drawFrame();
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
                    else return false;
                });
            }
            else frameInProgress.test_and_set(std::memory_order_relaxed);
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

            frameBegin = SDL_GetPerformanceCounter();
            targetDeltaTime = std::max(Application::secondsPerFrame - (deltaTime - targetDeltaTime), 0.0f);
            Time::frameDeltaTime = deltaTime * Time::timeScale;
        }

        #ifdef __EMSCRIPTEN__
        Debug::logTrace("Main loop end.");
        emscripten_sleep(std::max(unsigned(Application::secondsPerFrame * 1000.0f - 100.0f), 0u));
        #endif
    }
    
    for (auto it = SceneManager::existingScenes.begin(); it != SceneManager::existingScenes.end(); ++it)
        reinterpret_cast<Scene*>(it->data)->~Scene();
    SceneManager::existingScenes.clear();
    
    EngineEvent::OnQuit();

    while (Application::mainThreadQueue.try_dequeue(event));
    while (CoreEngine::pendingPreTickQueue.try_dequeue(event));
    while (CoreEngine::pendingPostTickQueue.try_dequeue(event));

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

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) [[unlikely]]
    {
        Debug::logError("Display initialisation failed! Error: ", SDL_GetError(), ".\n");
        throw "unimplemented";
        return;
    }

    if (!(CoreEngine::displMd = SDL_GetDesktopDisplayMode(1)))
    {
        Debug::logError("Failed to get desktop display details: ", SDL_GetError());
        throw "unimplemented";
    }

    Screen::width = CoreEngine::displMd->w;
    Screen::height = CoreEngine::displMd->h;
    Screen::screenRefreshRate = CoreEngine::displMd->refresh_rate;

    Window::width = Screen::width / 2;
    Window::height = Screen::height / 2;

    wind = SDL_CreateWindow("Window", Window::width, Window::height, SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (wind == nullptr)
    {
        Debug::logError("Could not create window: ", SDL_GetError(), ".");
        throw "unimplemented";
        return;
    }

    SDL_VERSION(&CoreEngine::backendVer);
    SDL_GetWindowWMInfo(CoreEngine::wind, &CoreEngine::wmInfo, SDL_SYSWM_CURRENT_VERSION);
    SDL_SetWindowMinimumSize(wind, 200, 200);

    struct {
        int32_t w, h;
        bool shouldUnlock = false;
        SpinLock shouldUnlockLock;
    } resizeData;

    SDL_AddEventWatch([](void* _data, SDL_Event* event) -> int
    // These few lines of code were literal years in the making. It formed part of my journey in learning C++.
    // I hope you fucking like your pretty rendering while resizing you straw assholes.
    {
        decltype(resizeData)& windowSizeData = *(decltype(resizeData)*)_data;

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
                int prevw = Window::width, prevh = Window::height;
                Window::width = w;
                Window::height = h;
                EngineEvent::OnWindowResize(Vector2Int { prevw, prevh });
            });
            
            renderResizeLock.lock();
        }

        return 0;
    }, &resizeData);

    SDL_SetWindowResizable(CoreEngine::wind, SDL_TRUE);
    
    CoreEngine::state.store(EngineState::RenderInit, std::memory_order_relaxed); // Spin off rendering thread.

    SDL_Event ev;
    while (true)
    {
        if (SDL_PeepEvents(&ev, 1, SDL_PEEKEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST) &&
            ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
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
                    Input::internalMousePosition.x = posX - Window::width / 2;
                    Input::internalMousePosition.y = -posY + Window::height / 2;
                    Input::internalMouseMotion.x += motX;
                    Input::internalMouseMotion.y += motY;
                    EngineEvent::OnMouseMove(Vector2Int(motX, -motY));
                });
                break;
            case SDL_EVENT_MOUSE_WHEEL:
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
                case true:
                    Application::mainThreadQueue.enqueue([key = Input::convertFromSDLKey(ev.key.keysym.sym)]
                    {
                        EngineEvent::OnKeyRepeat(key);
                    });
                    break;
                case false:
                    Application::mainThreadQueue.enqueue([key = Input::convertFromSDLKey(ev.key.keysym.sym)]
                    {
                        Input::heldKeyInputs[(size_t)key] = true;
                        EngineEvent::OnKeyDown(key);
                    });
                    break;
                }
                break;
            case SDL_EVENT_KEY_UP:
                Application::mainThreadQueue.enqueue([key = Input::convertFromSDLKey(ev.key.keysym.sym)]
                {
                    Input::heldKeyInputs[(size_t)key] = false;
                    EngineEvent::OnKeyUp(key);
                });
                break;
                #pragma endregion
            case SDL_EVENT_WINDOW_MOVED:
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            case SDL_EVENT_QUIT:
                while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::Playing)
                {
                    #if __EMSCRIPTEN__
                    emscripten_sleep(1);
                    #endif
                }
                goto Exit;
                break;
            }
        }

        #ifdef __EMSCRIPTEN__
        Debug::logTrace("Event loop end.");
        emscripten_sleep(1);
        #endif
    }

    Exit:
    CoreEngine::state.store(EngineState::ExitRequested, std::memory_order_relaxed);
    // Wait for render thread to finish.
    while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::RenderThreadDone)
    {
        #ifdef __EMSCRIPTEN__
        emscripten_sleep(1);
        #endif
    }

    SDL_DestroyWindow(CoreEngine::wind);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);

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
    RendererBackend backendPriorityOrder[]
    {
        #if _WIN32
        RendererBackend::OpenGL,
        RendererBackend::Vulkan,
        RendererBackend::Direct3D12,
        RendererBackend::Direct3D11,
        RendererBackend::Direct3D9
        #else
        RendererBackend::OpenGL,
        RendererBackend::Vulkan
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

    if (!Renderer::initialize
        (
            #if defined(SDL_ENABLE_SYSWM_X11)
            (void*)wmInfo.info.x11.display,
            #elif defined(SDL_ENABLE_SYSWM_WAYLAND)
            (void*)wmInfo.info.wl.display,
            #else
            nullptr,
            #endif

            #if __EMSCRIPTEN__
            (void*)"#canvas",
            #elif defined(SDL_ENABLE_SYSWM_X11)
            (void*)wmInfo.info.x11.window,
            #elif defined(SDL_ENABLE_SYSWM_WAYLAND)
            (void*)wmInfo.info.wl.egl_window,
            #elif defined(SDL_ENABLE_SYSWM_COCOA)
            (void*)wmInfo.info.cocoa.window,
            #elif defined(SDL_ENABLE_SYSWM_WINDOWS)
            (void*)wmInfo.info.win.window,
            #else
            nullptr,
            #endif

            Window::width, Window::height, initBackend
        ))
    {
        Debug::logError("goddamnit");
        throw "unimplemented";
    }
    
    Renderer::setViewArea(0, 0, 0, (uint16_t)Window::width, (uint16_t)Window::height);
    Renderer::setViewArea(1, 0, 0, (uint16_t)Window::width, (uint16_t)Window::height);
    Renderer::setViewOrthographic(1, float(Window::width), float(Window::height), Vector3(0, 0, 0), Renderer::fromEuler(Vector3(0, 0, 0)), -16777216.0f, 16777216.0f);
    Renderer::setViewClear(0, 0xffffffff, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
    
    CoreEngine::state.store(EngineState::RenderThreadReady, std::memory_order_relaxed); // Signal main thread.

    RenderJob job;
    while (CoreEngine::state.load(std::memory_order_relaxed) != EngineState::MainThreadDone)
    {
        while (renderQueue.try_dequeue(job))
        {
            if (renderQueue.size_approx() > GL_QUEUE_OVERBURDENED_THRESHOLD && !job.required)
                Debug::logInfo("Render queue overburdened, skipping render job id ", job.callFunction, ".\n");
            else job();
            job.destroy();
        }

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

    #ifdef __linux__
    #define __platform_linux 1
    #else
    #define __platform_linux 0
    #endif
    InternalEngineEvent::OnRenderShutdown();
    if (!((bool)__platform_linux && initBackend == RendererBackend::OpenGL))
        Renderer::shutdown();

    CoreEngine::state.store(EngineState::RenderThreadDone, std::memory_order_relaxed); // Signal window thread.
}
