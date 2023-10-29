#include "Task.h"

using namespace Firework;

std::vector<std::jthread> ThreadPool::threads = []
{
    std::vector<std::jthread> ret;
    ret.reserve(FIREWORK_THREAD_POOL_COUNT);
    for (size_t i = 0; i < FIREWORK_THREAD_POOL_COUNT; i++)
    {
        ret.push_back(std::jthread([](std::stop_token token)
        {
            func::function<void()> job;
            while (!token.stop_requested())
            {
                while (ThreadPool::jobs.try_dequeue(job))
                    job();
                std::this_thread::yield();
            }
        }));
    }
    return ret;
}();
moodycamel::ConcurrentQueue<func::function<void()>> ThreadPool::jobs;