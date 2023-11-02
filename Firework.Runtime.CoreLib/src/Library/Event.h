#pragma once

#include <set>
#include <function.h>

namespace Firework
{
    /// @brief Iterator alias for a subscribed event handler.
    /// @tparam ...Args Arguments passed by the event when it is raised.
    template <typename... Args>
    using EventHandler = typename std::set<func::function<void(Args...)>>::const_iterator;
    /// @brief Event capable of having any type of functor subscribed to it.
    /// @tparam ...Args Arguments passed by the event when it is raised.
    template <typename... Args>
    class Event
    {
        std::set<func::function<void(Args...)>> children;
    public:
        inline Event()
        { }
        Event(const Event<Args...>&) = delete;
        Event(Event<Args...>&&) = delete;

        /// @brief Subscribe a event handler to this event.
        /// @param func Event handler to subscribe.
        /// @return Iterator to the event handler, used to unsubscribe.
        /// @note Not thread-safe.
        inline EventHandler<Args...> operator+=(func::function<void(Args...)>&& func)
        {
            return this->children.emplace(std::forward(func)).first;
        }
        /// @brief Unsubscribe a event handler from this event.
        /// @param func Event handler to unsubscribe.
        /// @return Whether the event handler was unsubscribed.
        /// @retval - ```true```: The event handler was unsubscribed.
        /// @retval - ```false```: The event handler was not subscribed to this event.
        /// @note Not thread-safe.
        inline bool operator-=(const func::function<void(Args...)>& func)
        {
            if (auto it = this->children.find(func); it != this->children.end()) [[likely]]
            {
                this->children.erase(it);
                return true;
            }
            else return false;
        }
        /// @brief Unsubscribe an event handler from this event.
        /// @param it Iterator to event handler, returned by ```Firework::Event<...>::operator+=```.
        /// @note Not thread-safe.
        inline void operator-=(EventHandler<Args...> it)
        {
            this->children.erase(it);
        }

        /// @brief Raise this event.
        /// @param ...args Arguments to pass to subscribed event handlers.
        /// @note Not thread-safe.
        inline void operator()(Args... args)
        {
            for (auto it = this->children.begin(); it != this->children.end(); ++it)
                (*it)(args...);
        }

        /// @brief Retrieve whether this event is handled.
        /// @return Whether this event has any subscribed event handlers.
        /// @retval - ```true```: This event has no subscribed event handlers.
        /// @retval - ```false```: This event has at least one subscribed event handler.
        /// @note Not thread-safe.
        inline bool unhandled()
        {
            return this->children.empty();
        }
        /// @brief Remove all subscribed event handlers from this event.
        /// @note Not thread-safe.
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