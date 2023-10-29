#pragma once

#include <type_traits>

struct RenderJob
{
    void* function;
    void (*callFunction)(void*);
    void (*freeFunction)(void*);
    bool required;

    template <typename Func>
    inline static RenderJob create(Func&& func, bool required = true)
    {
        return RenderJob
        {
            new typename std::remove_cvref<Func>::type(func),
            [](void* function)
            {
                (*static_cast<typename std::remove_cvref<Func>::type*>(function))();
            },
            [](void* function)
            {
                delete static_cast<typename std::remove_cvref<Func>::type*>(function);
            },
            required
        };
    }
    inline void destroy() const
    {
        this->freeFunction(this->function);
    }

    inline void operator()() const
    {
        this->callFunction(this->function);
    }
};