#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

_push_nowarn_gcc(_clWarn_gcc_c_cast);
_push_nowarn_clang(_clWarn_clang_c_cast);
#include <SDL3/SDL.h>
#include <SDL3/SDL_version.h>
#include <atomic>
#include <concepts>
#include <concurrentqueue.h>
#include <deque>
#include <function.h>
#include <list>
#include <vector>
_pop_nowarn_clang();
_pop_nowarn_gcc();

#include <Core/RenderJob.h>

namespace Firework
{
    class Application;
    class Debug;

    class Window;
    class Cursor;

    class Input;
} // namespace Firework

_push_nowarn_msvc(_clWarn_msvc_export_interface);
namespace Firework::Internal
{
    /// @internal
    /// @brief Internal API. what the ~~dog~~ engine doin'
    enum class EngineState : uint_fast8_t
    {
        FirstInit,
        WindowInit,
        RenderInit,
        RenderThreadReady,
        Playing,
        ExitRequested,
        MainThreadDone,
        RenderThreadDone,
        WindowThreadDone
    };

    constexpr auto operator<=>(Firework::Internal::EngineState a, Firework::Internal::EngineState b)
    {
        return std::to_underlying(a) <=> std::to_underlying(b);
    }

    /// @internal
    /// @brief Static class containing functionality relevant to the backend operations of the runtime.
    class _fw_core_api CoreEngine final
    {
        static std::atomic<EngineState> state;

        static SDL_Window* wind;
        static SDL_Renderer* rend;
        static const SDL_DisplayMode* displMd;

        /// @internal
        /// @brief Internal API. Update the display information.
        /// @note Window thread only.
        static void resetDisplayData();

        static moodycamel::ConcurrentQueue<func::function<void()>> pendingPreTickQueue;
        static moodycamel::ConcurrentQueue<func::function<void()>> pendingPostTickQueue;

        static std::vector<RenderJob> frameRenderJobs;

        static std::deque<RenderJob> renderQueue;
        static std::mutex renderQueueLock;

        /// @internal
        /// @brief Internal API. The main thread loop. Blocks.
        /// @note Main thread only.
        static void internalLoop();
        /// @internal
        /// @brief Internal API. The window thread loop. Blocks.
        /// @note Window thread only.
        static void internalWindowLoop();
        /// @internal
        /// @brief Internal API. The render thread loop. Blocks.
        /// @note Render thread only.
        static void internalRenderLoop();

        /// @internal
        /// @brief Internal API. Initialize and start the runtime.
        /// @param argc Forwarded from int main(...).
        /// @param argv Forwarded from int main(...).
        /// @return Whether the runtime was able to successfully initialize.
        /// @retval - EXIT_SUCCESS: The runtime initialized successfully.
        /// @retval - EXIT_FAILIURE: The runtime failed to initialize.
        /// @note Thread-safe.
        static int execute(int argc, char* argv[]);
    public:
        CoreEngine() = delete;

        /// @internal
        /// @brief Low-level API.
        /// @tparam Func ```requires std::constructible_from<func::function<void()>, Func&&>```
        /// @param job Job to queue.
        /// @param required Whether this job has to run if the runtime is behind.
        /// @note Thread-safe.
        template <std::invocable<> Func>
        inline static void queueRenderJobForFrame(Func&& job, bool required = true)
        {
            std::lock_guard guard(CoreEngine::renderQueueLock);
            CoreEngine::renderQueue.push_back(RenderJob(job, required));
        }

        friend class Firework::Application;
        friend class Firework::Debug;

        friend class Firework::Input;
        friend class Firework::Window;
        friend class Firework::Cursor;
    };
} // namespace Firework::Internal
_pop_nowarn_msvc();
