#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <atomic>
#include <concurrentqueue.h>
#include <coroutine>
#include <function.h>
#include <memory>

#define FIREWORK_THREAD_POOL_COUNT 2

namespace Firework
{
    namespace Internal
    {
        class CoreEngine;
        struct ThreadPoolStaticInit;

        template <typename T>
        struct TaskPromise;
    }

    template <typename T>
    class Task;

    class __firework_corelib_api ThreadPool final
    {
        static std::vector<std::jthread> threads;
        static moodycamel::ConcurrentQueue<func::function<void()>> jobs;
    public:
        template <typename T>
        friend class Firework::Task;

        friend struct Firework::Internal::ThreadPoolStaticInit;
        friend class Firework::Internal::CoreEngine;
    };

    enum class TaskStatus : uint_fast8_t
    {
        Created = 0,
        WaitingToRun = 2,
        Running = 3,
        RanToCompletion = 5,
        Canceled = 6
    };

    template <typename T = void>
    class Task
    {
    public:
        using promise_type = Internal::TaskPromise<T>;

        inline explicit Task(std::coroutine_handle<Internal::TaskPromise<T>> handle) : handle(handle)
        { }
        inline Task(const Task& other) = delete;
        inline Task(Task&& other) : handle([&]
        {
            std::lock_guard guard(other.lock);
            std::coroutine_handle<Internal::TaskPromise<T>> ret = other.handle;
            other.handle = nullptr;
            return ret;
        }())
        { }
        inline ~Task()
        {
            this->handle.destroy();
        }

        inline decltype(auto) operator co_await()
        {
            std::lock_guard guard(this->lock);
            struct
            {
                std::coroutine_handle<Internal::TaskPromise<T>> handle;
                std::mutex& lock;

                inline bool await_ready()
                {
                    std::lock_guard guard(this->lock);
                    return !this->handle || this->handle.done();
                }
                inline void await_suspend(std::coroutine_handle<Internal::TaskPromise<T>> _handle)
                {
                    this->lock.lock();
                    if (_handle && this->handle)
                    {
                        if (_handle.promise().shouldRunSynchronously)
                        {
                            this->handle.promise().shouldRunSynchronously = true;
                            while (!this->handle.done())
                                this->handle.resume();
                        }
                        else
                        {
                            Task::pushToPool(this->handle, this->lock);
                            this->handle.promise().status = TaskStatus::WaitingToRun;
                            while (this->handle.promise().status != TaskStatus::RanToCompletion)
                            {
                                this->lock.unlock();
                                std::this_thread::yield();
                                this->lock.lock();
                            }
                        }
                    }
                    this->lock.unlock();
                }
                inline void await_resume()
                { }
            } ret { this->handle, this->lock };
            return ret;
        }

        inline void wait()
        {
            this->lock.lock();
            if (this->handle.promise().status == TaskStatus::Created)
            {
                this->handle.promise().shouldRunSynchronously = true;
                while (!this->handle.done())
                    this->handle.resume();
            }
            else while (!this->handle.done())
            {
                this->lock.unlock();
                std::this_thread::yield();
                this->lock.lock();
            }
            this->lock.unlock();
        }
        inline void runSynchronously()
        {
            std::coroutine_handle<Internal::TaskPromise<T>> handle = [this]
            {
                std::lock_guard guard(this->lock);
                std::coroutine_handle<Internal::TaskPromise<T>> ret = std::coroutine_handle<Internal::TaskPromise<T>>::from_promise(*this->promise);
                this->handle.promise().shouldRunSynchronously = true;
                return ret;
            }();
            while (!handle.done())
                handle.resume();
        }
    private:
        inline static void pushToPool(std::coroutine_handle<Internal::TaskPromise<T>> handle, std::mutex& lock)
        {
            ThreadPool::jobs.enqueue([handle, &lock]
            {
                lock.lock();
                handle.resume();
                if (!handle.done())
                {
                    lock.unlock();
                    Task::pushToPool(handle, lock);
                }
                else
                {
                    handle.promise().status = TaskStatus::RanToCompletion;
                    lock.unlock();
                }
            });
        }

        std::coroutine_handle<Internal::TaskPromise<T>> handle;
        std::mutex lock;
    };

    namespace Internal
    {
        template <typename T>
        struct Promise
        {
            TaskStatus status = TaskStatus::Created;
            bool shouldRunSynchronously = false;

            inline std::suspend_always initial_suspend()
            {
                return std::suspend_always();
            }
            inline std::suspend_always final_suspend() noexcept
            {
                return std::suspend_always();
            }
        };
        template <typename T>
        struct TaskPromise : public Promise<T>
        {
            alignas(T) unsigned char returnBuffer[sizeof(T)];

            inline Task<T> get_return_object()
            {
                return Task(std::coroutine_handle<Internal::TaskPromise<T>>::from_promise(*this));
            }

            template <typename Type>
            void return_value(Type&& value)
            {
                new(this->returnBuffer) T(std::forward(value));
            }
            void unhandled_exception()
            { }
        };
        template <>
        struct TaskPromise<void> : public Promise<void>
        {
            inline Task<void> get_return_object()
            {
                return Task(std::coroutine_handle<Internal::TaskPromise<void>>::from_promise(*this));
            }

            void return_void()
            { }
            void unhandled_exception()
            { }
        };
    }
}