#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <cstddef>
#include <memory>
#include <module/sys.Mathematics>
#include <module/sys>
#include <robin_hood.h>
#include <type_traits>

#include <Components/RectTransform.h>
#include <Core/Debug.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <Library/Hash.h>
#include <Library/TypeInfo.h>
#include <Objects/Component2D.h>
#include <typeindex>

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
        inline std::shared_ptr<T> fetchComponent()
        {
            auto& componentSet = Entities::table[std::type_index(typeid(T))];

            auto it = componentSet.find(this);
            if (it != componentSet.end())
            {
                if constexpr (Get)
                    return std::static_pointer_cast<T>(it->second);
                else
                    return nullptr;
            }

            if constexpr (Add)
                return componentSet.emplace(robin_hood::pair(this, std::make_shared<T>()))->first.second;
            else
                return nullptr;
        }
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
        inline std::shared_ptr<T> addComponent()
        {
            return this->fetchComponent<T, false, true>();
        }
        template <typename T>
        inline std::shared_ptr<T> getComponent()
        {
            return this->fetchComponent<T, true, false>();
        }
        template <typename T>
        inline std::shared_ptr<T> getOrAddComponent()
        {
            return this->fetchComponent<T, true, true>();
        }
        template <typename T>
        inline void removeComponent()
        {
            static_assert(!std::is_same<T, RectTransform>::value,
                          "Cannot remove component from Entity. You cannot remove RectTransform from an Entity, it is a default component.");
            static_assert(std::is_base_of<Internal::Component2D, T>::value, "Cannot get component from Entity. Typename \"T\" is not derived from type \"Component2D\"");

            auto it = EntityManager2D::components.find(std::make_pair(this, __typeid(T).qualifiedNameHash()));
            if (it != EntityManager2D::components.end())
            {
                delete it->second;
                EntityManager2D::components.erase(it);
            }
            else
                Debug::logWarn("No identical component could be found on this Entity to be removed!");
        }
    };
} // namespace Firework