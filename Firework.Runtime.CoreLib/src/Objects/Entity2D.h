#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <vector>
#include <type_traits>
#include <robin_hood.h>

#include <Mathematics.h>
#include <Core/Debug.h>
#include <Components/RectTransform.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <Objects/Component2D.h>
#include <Library/ManagedArray.h>
#include <Library/Hash.h>
#include <Library/TypeInfo.h>

#include <Objects/Entity2D.inc>

namespace Firework
{
    template <typename T>
    T* Entity2D::addComponent()
    {
        static_assert(!std::is_same<T, RectTransform>::value, "Cannot add component to Entity2D. You cannot add RectTransform to an Entity2D, it is a default component.");
        static_assert(std::is_base_of<Internal::Component2D, T>::value, "Cannot add component to Entity2D. Typename \"T\" is not derived from type \"Component2D\"");

        if (!EntityManager2D::components.contains(std::make_pair(this, __typeid(T).qualifiedNameHash())))
        {
            auto it = EntityManager2D::existingComponents.find(__typeid(T).qualifiedNameHash());
            if (it != EntityManager2D::existingComponents.end())
                ++it->second;
            else EntityManager2D::existingComponents.emplace(__typeid(T).qualifiedNameHash(), 1);

            T* ret = new T();
            EntityManager2D::components.emplace(std::make_pair(this, __typeid(T).qualifiedNameHash()), ret);
            ret->attachedEntity = this;
            ret->attachedRectTransform = this->attachedRectTransform;
            ret->reflection.typeID = __typeid(T).qualifiedNameHash();

            return ret;
        }
        else
        {
            Debug::LogError("Entity2D already has this type of component!");
            return nullptr;
        }
    }
    template <typename T>
    T* Entity2D::getComponent()
    {
        if constexpr (std::is_same<T, RectTransform>::value)
            return this->attachedRectTransform;
        else
        {
            static_assert(std::is_base_of<Internal::Component2D, T>::value, "Cannot get component from Entity2D. Typename \"T\" is not derived from type \"Component2D\"");

            auto it = EntityManager2D::components.find(std::make_pair(this, __typeid(T).qualifiedNameHash()));
            if (it != EntityManager2D::components.end())
                return it;

            Debug::LogWarn("No component of type \"", __typeid(T).qualifiedName(), "\" could be found on this Entity2D!");
            return nullptr;
        }
    }
    template <typename T>
    void Entity2D::removeComponent()
    {
        static_assert(!std::is_same<T, RectTransform>::value, "Cannot remove component from Entity2D. You cannot remove RectTransform from an Entity2D, it is a default component.");
        static_assert(std::is_base_of<Internal::Component2D, T>::value, "Cannot get component from Entity2D. Typename \"T\" is not derived from type \"Component2D\"");
        
        auto it = EntityManager2D::components.find(std::make_pair(this, __typeid(T).qualifiedNameHash()));
        if (it != EntityManager2D::components.end())
        {
            delete it->second;
            EntityManager2D::components.erase(it);
        }
        else Debug::LogWarn("No identical component could be found on this Entity2D to be removed!");
    }
}