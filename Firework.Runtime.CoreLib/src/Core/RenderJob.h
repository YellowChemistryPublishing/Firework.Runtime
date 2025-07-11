#pragma once

#include <concepts>
#include <function.h>
#include <type_traits>

/// @internal
/// @brief Internal API. Functor wrapper for a funtion of signature void(), used to submit render jobs.
/// @note Always pass by value.
struct RenderJob
{
    void* function;
    void (*callFunction)(void*);
    void (*freeFunction)(void*);
    bool required;

    /// @internal
    /// @brief Internal API. Creates a new RenderJob.
    /// @tparam Func ```requires requires { func::function<void()>(func); }```
    /// @param func Function to create job from.
    /// @param required Whether this job has to run if the runtime is behind.
    /// @return Render job that will call the given function.
    template <std::invocable<> Func>
    inline static RenderJob create(Func&& func, bool required = true)
    {
        return RenderJob { new typename std::remove_cvref<Func>::type(func), [](void* function) { (*static_cast<typename std::remove_cvref<Func>::type*>(function))(); },
                           [](void* function) { delete static_cast<typename std::remove_cvref<Func>::type*>(function); }, required };
    }
    /// @internal
    /// @brief Internal API. Destroys this RenderJob.
    inline void destroy() const
    {
        this->freeFunction(this->function);
    }

    /// @internal
    /// @brief Internal API. Runs the render job.
    inline void operator()() const
    {
        this->callFunction(this->function);
    }
};