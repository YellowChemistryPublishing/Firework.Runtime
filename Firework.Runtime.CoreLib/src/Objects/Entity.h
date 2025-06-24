#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <module/sys.Mathematics>
#include <robin_hood.h>
#include <type_traits>
#include <vector>

#include <Core/Debug.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <Library/Hash.h>
#include <Library/TypeInfo.h>
#include <Objects/Component.h>
#include <Objects/Entity.inc>

namespace Firework
{
    template <typename T>
    requires (!std::same_as<T, Transform> && std::derived_from<T, Internal::Component>)
    T* Entity::addComponent()
    {
        if (!EntityManager::components.contains(std::make_pair(this, __typeid(T).qualifiedNameHash())))
        {
            T* ret = new T();

            EntityManager::components.emplace(std::make_pair(this, __typeid(T).qualifiedNameHash()), ret);
            ret->attachedEntity = this;
            ret->attachedTransform = this->attachedTransform;
            ret->reflection.typeID = __typeid(T).qualifiedNameHash();

            auto it = EntityManager::existingComponents.find(__typeid(T).qualifiedNameHash());
            if (it != EntityManager::existingComponents.end())
                ++it->second;
            else
                EntityManager::existingComponents.emplace(__typeid(T).qualifiedNameHash(), 1);

            if constexpr (requires { ret->onCreate(); })
                ret->onCreate();

            return ret;
        }
        else
        {
            Debug::logError("Entity already has this type of component!");
            return nullptr;
        }
    }
    template <typename T>
    requires std::derived_from<T, Internal::Component>
    T* Entity::getComponent()
    {
        if constexpr (std::is_same<T, Transform>::value)
            return this->attachedTransform;
        else
        {
            auto it = EntityManager::components.find({ this->attachedScene, this, __typeid(T).qualifiedNameHash() });
            if (it != EntityManager::components.end())
                return it;

            Debug::logWarn("No component of type \"", __typeid(T).qualifiedName(), "\" could be found on this Entity!");
            return nullptr;
        }
    }
    template <typename T>
    requires (!std::same_as<T, Transform> && std::derived_from<T, Internal::Component>)
    void Entity::removeComponent()
    {
        auto it = EntityManager::components.find({ this->attachedScene, this, __typeid(T).qualifiedNameHash() });
        if (it != EntityManager::components.end())
        {
            delete it->second;
            EntityManager::components.erase(it);
        }
        else
            Debug::logWarn("No identical component could be found on this Entity to be removed!");
    }
} // namespace Firework