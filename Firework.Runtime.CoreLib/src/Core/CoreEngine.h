#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

_push_nowarn_c_cast();
#include <SDL3/SDL.h>
#include <SDL3/SDL_version.h>
#include <atomic>
#include <concepts>
#include <concurrentqueue.h>
#include <deque>
#include <function.h>
#include <list>
#include <vector>
_pop_nowarn_c_cast();

#include <Core/RenderJob.h>

namespace Firework
{
    class Application;
    class Debug;

    class Cursor;
    class Input;
    class Window;
    class Display;
} // namespace Firework

_push_nowarn_msvc(_clWarn_msvc_export_interface);
namespace Firework::Internal
{
    /// @internal
    /// @brief Internal API. what the ~~dog~~ engine doin'
    enum class EngineState : uint_fast8_t
    {
        WindowInit,
        RenderInit,
        RenderThreadReady,
        Running,
        ExitRequested,
        MainThreadDone,
        RenderThreadDone,
        WindowThreadDone,
        Count
    };
    constexpr auto operator<=>(EngineState a, EngineState b)
    {
        return std::to_underlying(a) <=> std::to_underlying(b);
    }

    /// @internal
    /// @brief Static class containing functionality relevant to the backend operations of the runtime.
    class _fw_core_api CoreEngine final
    {
        static SDL_Window* wind;
        static SDL_Renderer* rend;
        static const SDL_DisplayMode* displMd;

        static std::atomic_flag state[size_t(EngineState::Count)];

        static std::deque<RenderJob> renderQueue;
        static std::mutex renderQueueLock;
        static std::atomic<uint_least8_t> framesInFlight;

        inline static void waitSome(std::chrono::nanoseconds durationHint = Config::UnspecifiedSleepDuration)
        {
            if constexpr (Config::LatencyTrade == Config::LatencyTradeSetting::ThreadYield)
                std::this_thread::yield();
            else if constexpr (Config::LatencyTrade == Config::LatencyTradeSetting::ThreadSleep)
                std::this_thread::sleep_for(Config::UnspecifiedSleepDuration);
            else // if constexpr (Config::LatencyTrade == Config::LatencyTradeSetting::None)
            {
                // Busy wait.
            }
        }

        /// @internal
        /// @brief Internal API. Update the display information.
        /// @note Window thread only.
        static void resetDisplayData();

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
        /// @retval - EXIT_FAILURE: The runtime failed to initialize.
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
            CoreEngine::renderQueue.emplace_back(std::forward<Func>(job), required);
        }

        friend class Firework::Application;
        friend class Firework::Debug;

        friend class Firework::Cursor;
        friend class Firework::Input;
        friend class Firework::Window;
        friend class Firework::Display;
    };
} // namespace Firework::Internal
_pop_nowarn_msvc();
