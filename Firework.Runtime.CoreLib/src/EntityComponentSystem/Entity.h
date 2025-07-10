#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <cstddef>
#include <memory>
#include <module/sys.Mathematics>
#include <module/sys>
#include <robin_hood.h>
#include <type_traits>
#include <typeindex>

#include <EntityComponentSystem/EntityManagement.inc>
#include <Library/Hash.h>
#include <Library/Property.h>
#include <Library/TypeInfo.h>

namespace Firework
{
    class Entity;
    class EntityManager;

    struct EntityIterator
    {
        using difference_type = ptrdiff_t;
        using value_type = Entity;

        EntityIterator() = default;
        EntityIterator(std::shared_ptr<Entity> entity) : current(std::move(entity))
        { }

        inline value_type& operator*()
        {
            return *this->current;
        }
        inline value_type* operator->()
        {
            return this->current.get();
        }

        constexpr friend bool operator==(const EntityIterator&, const EntityIterator&) = default;

        inline EntityIterator& operator++();
        EntityIterator operator++(int)
        {
            EntityIterator ret = *this;
            ++*this;
            return ret;
        }
        inline EntityIterator& operator--();
        EntityIterator operator--(int)
        {
            EntityIterator ret = *this;
            --*this;
            return ret;
        }
    private:
        std::shared_ptr<Entity> current = nullptr;
    };
    struct EntityRange
    {
        inline EntityIterator begin()
        {
            return EntityIterator(this->front);
        }
        inline EntityIterator end()
        {
            return EntityIterator();
        }

        bool empty()
        {
            return !this->front;
        }

        EntityRange(std::shared_ptr<Entity> front) : front(std::move(front))
        { }
    private:
        std::shared_ptr<Entity> front = nullptr;
    };

    class __firework_corelib_api Entity final : public std::enable_shared_from_this<Entity>
    {
        std::shared_ptr<Entity> next = nullptr;
        std::shared_ptr<Entity> prev = nullptr;

        std::shared_ptr<Entity> _parent = nullptr;
        std::shared_ptr<Entity> _childrenFront = nullptr;
        std::shared_ptr<Entity> _childrenBack = nullptr;

        Entity() noexcept = default;

        void orphan() noexcept;
        void reparentAfterOrphan(std::shared_ptr<Entity> parent) noexcept;

        template <typename T, bool Get, bool Add>
        requires (Get || Add)
        inline std::shared_ptr<T> fetchComponent();
    public:
        const Property<std::shared_ptr<Entity>, std::shared_ptr<Entity>> parent { [this]() -> std::shared_ptr<Entity> { return this->_parent; },
                                                                                  [this](std::shared_ptr<Entity> value)
        {
            std::shared_ptr<Entity> lease = shared_from_this();
            this->orphan();
            this->reparentAfterOrphan(value);
        } };

        static std::shared_ptr<Entity> alloc(std::shared_ptr<Entity> parent = nullptr);
        ~Entity();
        void clear();

        inline EntityIterator childrenBegin()
        {
            return EntityIterator(this->_childrenFront);
        }
        inline EntityIterator childrenEnd()
        {
            return EntityIterator();
        }
        inline EntityRange children()
        {
            return EntityRange(this->_childrenFront);
        }

        template <typename T>
        inline std::shared_ptr<T> addComponent();
        template <typename T>
        inline std::shared_ptr<T> getComponent();
        template <typename T>
        inline std::shared_ptr<T> getOrAddComponent();
        template <typename T>
        inline bool removeComponent();

        friend struct Firework::EntityIterator;
        friend class Firework::Internal::CoreEngine;
    };

    EntityIterator& EntityIterator::operator++()
    {
        this->current = this->current->next;
        return *this;
    }
    EntityIterator& EntityIterator::operator--()
    {
        this->current = this->current->prev;
        return *this;
    }

    template <typename T, bool Get, bool Add>
    requires (Get || Add)
    std::shared_ptr<T> Entity::fetchComponent()
    {
        auto componentSetIt = Entities::table.emplace(std::type_index(typeid(T)), robin_hood::unordered_map<Entity*, std::shared_ptr<void>>()).first;
        sys::sc_act contractKeeper([&]
        {
            if (componentSetIt->second.empty()) [[unlikely]]
                Entities::table.erase(componentSetIt);
        });

        auto it = componentSetIt->second.find(this);
        if (it != componentSetIt->second.end())
        {
            if constexpr (Get)
                return std::static_pointer_cast<T>(it->second);
            else
                return nullptr;
        }

        if constexpr (Add)
        {
            std::shared_ptr<T> ret = std::make_shared<T>();
            componentSetIt->second.emplace(robin_hood::pair(this, std::static_pointer_cast<void>(ret)));
            if constexpr (requires { ret->onAttach(*this); })
                ret->onAttach(*this);
            return ret;
        }
        else
            return nullptr;
    }

    template <typename T>
    std::shared_ptr<T> Entity::addComponent()
    {
        return this->fetchComponent<T, false, true>();
    }
    template <typename T>
    std::shared_ptr<T> Entity::getComponent()
    {
        return this->fetchComponent<T, true, false>();
    }
    template <typename T>
    std::shared_ptr<T> Entity::getOrAddComponent()
    {
        return this->fetchComponent<T, true, true>();
    }

    template <typename T>
    bool Entity::removeComponent()
    {
        auto componentSetIt = Entities::table.find(std::type_index(typeid(T)));
        if (componentSetIt == Entities::table.end()) [[unlikely]]
            return false;

        auto componentIt = componentSetIt->second.find(this);
        if (componentIt == componentSetIt->second.end())
            return false;
        if (componentIt->second.use_count() > 1)
            return false;

        componentSetIt->second.erase(componentIt);
        return true;
    }
} // namespace Firework
