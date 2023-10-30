#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <vector>
#include <type_traits>
#include <robin_hood.h>

#include <Mathematics.h>
#include <Core/Debug.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <Objects/Component.h>
#include <Library/ManagedArray.h>
#include <Library/Hash.h>
#include <Library/TypeInfo.h>

#include <Objects/Entity.inc>

namespace Firework
{
    template <typename T>
    T* Entity::addComponent()
    {
        static_assert(!std::is_same<T, Transform>::value, "Cannot add component to Entity. You cannot add Transform to an Entity, it is a default component.");
        static_assert(std::is_base_of<Internal::Component, T>::value, "Cannot add component to Entity. Typename \"T\" is not derived from type \"Component\"");

        if (!EntityManager::components.contains(std::make_pair(this, __typeid(T).qualifiedNameHash())))
        {
            auto it = EntityManager::existingComponents.find(__typeid(T).qualifiedNameHash());
            if (it != EntityManager::existingComponents.end())
                ++it->second;
            else EntityManager::existingComponents.emplace(__typeid(T).qualifiedNameHash(), 1);

            T* ret = new T();
            EntityManager::components.emplace(std::make_pair(this, __typeid(T).qualifiedNameHash()), ret);
            ret->attachedEntity = this;
            ret->attachedTransform = this->attachedTransform;
            ret->reflection.typeID = __typeid(T).qualifiedNameHash();

            return ret;
        }
        else
        {
            Debug::logError("Entity already has this type of component!");
            return nullptr;
        }
    }
    template <typename T>
    T* Entity::getComponent()
    {
        if constexpr (std::is_same<T, Transform>::value)
            return this->attachedTransform;
        else
        {
            static_assert(std::is_base_of<Internal::Component, T>::value, "Cannot get component from Entity. Typename \"T\" is not derived from type \"Component\"");

            auto it = EntityManager::components.find({ this->attachedScene, this, __typeid(T).qualifiedNameHash() });
            if (it != EntityManager::components.end())
                return it;

            Debug::logWarn("No component of type \"", __typeid(T).qualifiedName(), "\" could be found on this Entity!");
            return nullptr;
        }
    }
    template <typename T>
    void Entity::removeComponent()
    {
        static_assert(!std::is_same<T, Transform>::value, "Cannot remove component from Entity. You cannot remove Transform from an Entity, it is a default component.");
        static_assert(std::is_base_of<Internal::Component, T>::value, "Cannot get component from Entity. Typename \"T\" is not derived from type \"Component\"");
        
        auto it = EntityManager::components.find({ this->attachedScene, this, __typeid(T).qualifiedNameHash() });
        if (it != EntityManager::components.end())
        {
            delete it->second;
            EntityManager::components.erase(it);
        }
        else Debug::logWarn("No identical component could be found on this Entity to be removed!");
    }
}