#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <concurrentqueue.h>
#include <function.h>
#include <glm/vec2.hpp>

#include <Library/Property.h>

namespace Firework::Internal
{
    class ShaderUtility;
    class CoreEngine;
} // namespace Firework::Internal

namespace Firework::PackageSystem
{
    class PackageManager;
}

_push_nowarn_msvc(_clWarn_msvc_export_interface);
namespace Firework
{
    class Image;

    /// @brief Static class containing functionality relevant to the currently running program.
    class _fw_core_api Application final
    {
        static moodycamel::ConcurrentQueue<func::function<void()>> mainThreadQueue;
        static moodycamel::ConcurrentQueue<func::function<void()>> windowThreadQueue;

        static std::atomic_flag workerThreadQueueNotif;
        static moodycamel::ConcurrentQueue<func::function<void()>> workerThreadQueue;

        static float secondsPerFrame;
    public:
        Application() = delete;

        /// @internal
        /// @brief Low-level API [Internal]. Queues a function to be run on the main thread.
        /// @tparam Func ```requires std::invocable<Func>```
        /// @param job Job to queue.
        /// @note Thread-safe.
        template <std::invocable<> Func>
        inline static void queueJobForMainThread(Func&& job)
        {
            Application::mainThreadQueue.enqueue(job);
        }
        /// @internal
        /// @brief Low-level API [Internal]. Queues a function to be run on the background worker thread.
        /// @tparam Func ```requires std::invocable<Func>```
        /// @param job Job to queue.
        /// @note Thread-safe.
        template <std::invocable<> Func>
        inline static void queueJobForWorkerThread(Func&& job)
        {
            Application::workerThreadQueue.enqueue(job);
            Application::workerThreadQueueNotif.test_and_set();
            Application::workerThreadQueueNotif.notify_one();
        }
        /// @internal
        /// @brief Low-level API [Internal]. Queues a function to be run on the window thread.
        /// @tparam Func ```requires std::invocable<Func>```
        /// @param job Job to queue.
        /// @note Thread-safe.
        template <std::invocable<> Func>
        inline static void queueJobForWindowThread(Func&& job)
        {
            Application::windowThreadQueue.enqueue(job);
        }

        /// @brief Sets the minimum frame time by frames-per-second.
        /// @param fps Framerate in frames per second.
        /// @note Main thread only.
        inline static void setTargetFrameRate(float fps)
        {
            Application::secondsPerFrame = 1.0f / fps;
        }
        /// @brief Sets the minimum frame time.
        /// @param deltaTime Frame time in seconds.
        /// @note Main thread only.
        inline static void setTargetDeltaTime(float deltaTime)
        {
            Application::secondsPerFrame = deltaTime;
        }

        /// @brief Start the runtime, blocking until the runtime has been requested to exit.
        /// @warning You shouldn't have to call this unless your main function is unmanaged!
        /// @warning Don't call this more than once!
        /// @param argc Forwarded from int main(...).
        /// @param argv Forwarded from int main(...).
        /// @return Whether the runtime was able to start successfully.
        /// @retval - `EXIT_SUCCESS`: The runtime initialized successfully.
        /// @retval - `EXIT_FAILIURE`: The runtime failed to initialize.
        /// @note Thread-safe.
        static int run(int argc, char* argv[]);

        /// @brief Politely request that the runtime should exit.
        /// @note Main thread only.
        static void quit();

        friend class Firework::Internal::CoreEngine;
    };
} // namespace Firework
_pop_nowarn_msvc();
