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
#include <EntityComponentSystem/Component.h>
#define FIREWORK_ENTITY_MGMT_DECL_ONLY 1
#include <EntityComponentSystem/EntityManagement.h>
#undef FIREWORK_ENTITY_MGMT_DECL_ONLY
#include <Library/Hash.h>
#include <Library/TypeInfo.h>

namespace Firework
{
    class Scene;
    class SceneManager;
    class Entity;
    class EntityManager;
    class Debug;

    namespace Internal
    {
        class CoreEngine;
    }

    class __firework_corelib_api Entity final : public Internal::Object
    {
        Scene* attachedScene;

        Entity* next = nullptr;
        Entity* prev = nullptr;

        Entity* _parent = nullptr;
        Entity* _childrenFront = nullptr;
        Entity* _childrenBack = nullptr;

        void setParent(Entity* value);

        template <typename T, bool Get, bool Add>
        requires (Get || Add)
        inline std::shared_ptr<T> fetchComponent();
    public:
        Entity();
        Entity(Entity* parent);
        ~Entity() override;

        const Property<Entity*, Entity*> parent { [this]() -> Entity* { return this->_parent; }, [this](Entity* value)
        {
            this->setParent(value);
        } };

        void moveBefore(Entity* entity);
        void moveAfter(Entity* entity);

        template <typename T>
        inline std::shared_ptr<T> addComponent();
        template <typename T>
        inline std::shared_ptr<T> getComponent();
        template <typename T>
        inline std::shared_ptr<T> getOrAddComponent();
        template <typename T>
        inline bool removeComponent();

        friend class Firework::Internal::CoreEngine;
        friend class Firework::RectTransform;
    };

#if !defined(FIREWORK_ENTITY_DECL_ONLY) || !FIREWORK_ENTITY_DECL_ONLY
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
            return componentSetIt->second.emplace(robin_hood::pair(this, std::make_shared<T>()))->first.second;
        else
            return nullptr;
    }

    template <typename T>
    inline std::shared_ptr<T> Entity::addComponent()
    {
        return this->fetchComponent<T, false, true>();
    }
    template <typename T>
    inline std::shared_ptr<T> Entity::getComponent()
    {
        return this->fetchComponent<T, true, false>();
    }
    template <typename T>
    inline std::shared_ptr<T> Entity::getOrAddComponent()
    {
        return this->fetchComponent<T, true, true>();
    }

    template <typename T>
    inline bool Entity::removeComponent()
    {
        auto componentSetIt = Entities::table.find(std::type_index(typeid(T)));
        if (componentSetIt == Entities::table.end()) [[unlikely]]
            return false;

        return componentSetIt->second.erase(this);
    }
#endif
} // namespace Firework