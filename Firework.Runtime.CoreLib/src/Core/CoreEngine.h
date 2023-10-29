#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <atomic>
#include <list>
#include <SDL3/SDL.h>
#include <SDL3/SDL_syswm.h>
#include <concurrentqueue.h>
#include <function.h>

#include <Core/RenderJob.h>
#include <Library/Lock.h>

namespace Firework
{
	class Application;

	namespace Internal
	{
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

		class __firework_corelib_api CoreEngine final
		{
			static std::atomic<EngineState> state;

			static SDL_Window* wind;
			static SDL_Renderer* rend;
			static const SDL_DisplayMode* displMd;
			static SDL_SysWMinfo wmInfo;
			static SDL_version backendVer;

			static void resetDisplayData();

			static moodycamel::ConcurrentQueue<func::function<void()>> pendingPreTickQueue;
			static moodycamel::ConcurrentQueue<func::function<void()>> pendingPostTickQueue;
			static moodycamel::ConcurrentQueue<RenderJob> renderQueue;
			static std::vector<RenderJob> frameRenderJobs;
			
			static void internalLoop();
			static void internalWindowLoop();
			static void internalRenderLoop();

			static int execute(int argc, char* argv[]);
		public:
			CoreEngine() = delete;

			template <typename Func>
			inline static void queueRenderJobForFrame(Func&& job, bool required = true)
			{
				CoreEngine::frameRenderJobs.push_back(RenderJob::create(job, required));
			}

			friend int __handleInitializeAndExit(int argc, char* argv[]);
			friend class Firework::Application;
		};
	}
}
