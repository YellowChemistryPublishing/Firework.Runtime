#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <concepts>
#include <concurrentqueue.h>
#include <filesystem>
#include <function.h>
#include <source_location>

void __fw_rt_trace_capture_stack(const char*, std::source_location, std::source_location);

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

    class __firework_corelib_api Application final
    {
        static moodycamel::ConcurrentQueue<func::function<void()>> mainThreadQueue;
        static float secondsPerFrame;
    public:
        template <typename Func>
        inline static void queueJobForMainThread(Func&& job)
        {
            Application::mainThreadQueue.enqueue(job);
        }

        inline static void setTargetFrameRate(float fps)
        {
            Application::secondsPerFrame = 1.0f / fps;
        }
        inline static void setTargetDeltaTime(float deltaTime)
        {
            Application::secondsPerFrame = deltaTime;
        }

        static int run(int argc, char* argv[]);
        static void quit();

        friend class Firework::Internal::CoreEngine;
        friend void ::__fw_rt_trace_capture_stack(const char*, std::source_location, std::source_location);
    };
}