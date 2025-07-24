#pragma once

#include <concepts>
#include <function.h>
#include <type_traits>
#include <utility>

namespace Firework
{
    /// @internal
    /// @brief Internal API. Functor wrapper for a funtion of signature void(), used to submit render jobs.
    /// @note Always pass by value.
    struct RenderJob
    {
        RenderJob() = default;
        /// @internal
        /// @brief Internal API. Creates a new RenderJob.
        /// @tparam Func ```requires requires { func::function<void()>(func); }```
        /// @param func Function to create job from.
        /// @param required Whether this job has to run if the runtime is behind.
        /// @return Render job that will call the given function.
        template <std::invocable<> Func>
        RenderJob(Func&& func, bool required = true) : func(func), _required(required)
        { }
        RenderJob(const RenderJob&) = default;
        RenderJob(RenderJob&& other)
        {
            swap(*this, other);
        }

        RenderJob& operator=(const RenderJob&) = default;
        RenderJob& operator=(RenderJob&& other)
        {
            swap(*this, other);
            return *this;
        }

        bool required() const
        {
            return this->_required;
        }

        const func::function<void()>& function() const
        {
            return this->func;
        }
        /// @internal
        /// @brief Internal API. Runs the render job.
        inline void operator()() const
        {
            this->func();
        }

        friend void swap(RenderJob& a, RenderJob& b)
        {
            using func::swap;
            using std::swap;

            swap(a.func, b.func);
            swap(a._required, b._required);
        }
    private:
        func::function<void()> func;
        bool _required = false;
    };
} // namespace Firework
