#pragma once

#include <set>
#include <function.h>

namespace Firework
{
    template <typename... Args>
    using EventHandler = typename std::set<func::function<void(Args...)>>::const_iterator;
    template <typename... Args>
    class Event
    {
        std::set<func::function<void(Args...)>> children;
    public:
        inline Event()
        {
        }
        Event(const Event<Args...>&) = delete;
        Event(Event<Args...>&&) = delete;

        inline EventHandler<Args...> operator+=(const func::function<void(Args...)>& func)
        {
            return this->children.emplace(func).first;
        }
        inline EventHandler<Args...> operator+=(func::function<void(Args...)>&& func)
        {
            return this->children.emplace(func).first;
        }
        inline bool operator-=(const func::function<void(Args...)>& func)
        {
            if (auto it = this->children.find(func); it != this->children.end()) [[likely]]
            {
                this->children.erase(it);
                return true;
            }
            else return false;
        }
        inline void operator-=(EventHandler<Args...> it)
        {
            this->children.erase(it);
        }
        inline void operator()(Args... args)
        {
            for (auto it = this->children.begin(); it != this->children.end(); ++it)
                (*it)(args...);
        }

        inline bool unhandled()
        {
            return this->children.empty();
        }
        inline void clear()
        {
            this->children.clear();
        }
    };
    
    template <typename... Args>
    class FuncPtrEvent
    {
        std::set<void (*)(Args...)> children;
    public:
        inline FuncPtrEvent()
        { }
        FuncPtrEvent(const FuncPtrEvent<Args...>&) = delete;
        FuncPtrEvent(FuncPtrEvent<Args...>&&) = delete;

        inline bool operator+=(void (*func)(Args...))
        {
            return this->children.emplace(func).second;
        }
        inline bool operator-=(void (*func)(Args...))
        {
            if (auto it = this->children.find(func); it != this->children.end()) [[likely]]
            {
                this->children.erase(it);
                return true;
            }
            else return false;
        }
        inline void operator()(Args... args)
        {
            for (auto it = this->children.begin(); it != this->children.end(); ++it)
                (*it)(std::forward<Args>(args)...);
        }

        inline bool unhandled()
        {
            return this->children.empty();
        }
        inline void clear()
        {
            this->children.clear();
        }
    };
}