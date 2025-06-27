#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <cstddef>
#include <memory>
#include <module/sys.Mathematics>
#include <module/sys>
#include <robin_hood.h>
#include <type_traits>
#include <typeindex>

#include <Components/RectTransform.h>
#include <Core/Debug.h>
#include <EntityComponentSystem/EntityManagement.inc>
#include <Library/Hash.h>
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
        EntityIterator(Entity* entity) : current(entity)
        { }

        inline value_type& operator*()
        {
            return *this->current;
        }
        inline value_type* operator->()
        {
            return this->current;
        }

        constexpr friend bool operator==(const EntityIterator& a, const EntityIterator& b) = default;

        inline EntityIterator& operator++();
        inline EntityIterator operator++(int)
        {
            EntityIterator ret = *this;
            ++*this;
            return ret;
        }
        inline EntityIterator& operator--();
        inline EntityIterator operator--(int)
        {
            EntityIterator ret = *this;
            --*this;
            return ret;
        }
    private:
        Entity* current = nullptr;
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

        EntityRange(Entity* front) : front(front)
        { }
    private:
        Entity* front = nullptr;
    };

    class __firework_corelib_api Entity final : std::enable_shared_from_this<Entity>
    {
        Entity* next = nullptr;
        Entity* prev = nullptr;

        Entity* _parent = nullptr;
        Entity* _childrenFront = nullptr;
        Entity* _childrenBack = nullptr;

        void orphan();
        void reparentAfterOrphan(Entity* parent);

        template <typename T, bool Get, bool Add>
        requires (Get || Add)
        inline std::shared_ptr<T> fetchComponent();
    public:
        Entity(Entity* parent = nullptr);
        ~Entity();

        const Property<Entity*, Entity*> parent { [this]() -> Entity* { return this->_parent; }, [this](Entity* value)
        {
            this->orphan();
            this->reparentAfterOrphan(value);
        } };

        EntityIterator childrenBegin()
        {
            return EntityIterator(this->_childrenFront);
        }
        EntityIterator childrenEnd()
        {
            return EntityIterator();
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
        friend class Firework::RectTransform;
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
            return std::static_pointer_cast<T>(componentSetIt->second.emplace(robin_hood::pair(this, std::static_pointer_cast<void>(std::make_shared<T>()))).first->second);
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

        return componentSetIt->second.erase(this);
    }
} // namespace Firework
