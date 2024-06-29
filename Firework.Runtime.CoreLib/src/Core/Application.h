#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <concurrentqueue.h>
#include <function.h>
#include <Mathematics.h>
#include <Library/Property.h>

namespace Firework
{
    class Image;

    namespace Internal
    {
        class ShaderUtility;
        class CoreEngine;
    }

    namespace PackageSystem
    {
        class PackageManager;
    }

    struct RuntimeInitializationOptions
    {
        std::string windowName = "Program";
        bool windowResizeable = false;
        Mathematics::Vector2Int resolution = Mathematics::Vector2Int(800, 600);
    };

    /// @brief Static class containing functionality relevant to the currently running program.
    class __firework_corelib_api Application final
    {
        static moodycamel::ConcurrentQueue<func::function<void()>> mainThreadQueue;
        static moodycamel::ConcurrentQueue<func::function<void()>> workerThreadQueue;
        static moodycamel::ConcurrentQueue<func::function<void()>> windowThreadQueue;

        static RuntimeInitializationOptions _initializationOptions;
        
        static float secondsPerFrame;
    public:
        Application() = delete;

        /// @internal
        /// @brief Low-level API [Internal]. Queues a function to be run on the main thread.
        /// @tparam Func ```requires std::constructible_from<func::function<void()>, Func&&>```
        /// @param job Job to queue.
        /// @note Thread-safe.
        template <typename Func>
		requires std::constructible_from<func::function<void()>, Func&&>
        inline static void queueJobForMainThread(Func&& job)
        {
            Application::mainThreadQueue.enqueue(job);
        }
        /// @internal
        /// @brief Low-level API [Internal]. Queues a function to be run on the background worker thread.
        /// @tparam Func ```requires std::constructible_from<func::function<void()>, Func&&>```
        /// @param job Job to queue.
        /// @note Thread-safe.
        template <typename Func>
		requires std::constructible_from<func::function<void()>, Func&&>
        inline static void queueJobForWorkerThread(Func&& job)
        {
            Application::workerThreadQueue.enqueue(job);
        }
        /// @internal
        /// @brief Low-level API [Internal]. Queues a function to be run on the window thread.
        /// @tparam Func ```requires std::constructible_from<func::function<void()>, Func&&>```
        /// @param job Job to queue.
        /// @note Thread-safe.
        template <typename Func>
		requires std::constructible_from<func::function<void()>, Func&&>
        inline static void queueJobForWindowThread(Func&& job)
        {
            Application::windowThreadQueue.enqueue(job);
        }

        static Property<const RuntimeInitializationOptions&, RuntimeInitializationOptions> initializationOptions;

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
        /// @retval - EXIT_SUCCESS: The runtime initialized successfully.
        /// @retval - EXIT_FAILIURE: The runtime failed to initialize.
        /// @note Thread-safe.
        static int run(int argc, char* argv[]);

        /// @brief Politely request that the runtime should exit.
        /// @note Main thread only.
        static void quit();

        friend class Firework::Internal::CoreEngine;
    };
}