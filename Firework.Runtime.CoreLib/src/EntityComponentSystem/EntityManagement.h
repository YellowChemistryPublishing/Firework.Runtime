#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <initializer_list>
#include <list>
#include <tuple>
#include <robin_hood.h>
#include <EntityComponentSystem/SceneManagement.h>
#include <Objects/Entity2D.inc>
#include <Objects/Entity.inc>
#include <Library/MinAllocList.h>
#include <Library/TupleHash.h>

namespace Firework
{
    class Scene;

    class Entity2D;
    class Entity;

    namespace Internal
    {
        class Component2D;
        class CoreEngine;
        
        template <typename EntityType>
        requires std::same_as<EntityType, Entity2D> || std::same_as<EntityType, Entity>
        struct EntityComponentHash // Specialization appears before first use.
        {
            size_t operator()(const std::pair<EntityType*, uint64_t>& value) const
            {
                return
                std::hash<EntityType*>()(value.first) ^
                std::hash<uint64_t>()(value.second);
            }
        };
    }

    class __firework_corelib_api EntityManager2D final
    {
        static robin_hood::unordered_map<uint64_t, uint64_t> existingComponents;
        static robin_hood::unordered_map<std::pair<Entity2D*, uint64_t>, Internal::Component2D*, Internal::EntityComponentHash<Entity2D>> components;
    public:
        EntityManager2D() = delete;

        template <typename Func>
        inline static void foreachEntity(const Func&& func)
        {
            for
            (
                auto _it1 = SceneManager::existingScenes.begin();
                _it1 != SceneManager::existingScenes.end();
                ++_it1
            )
            {
                Scene* it1 = reinterpret_cast<Scene*>(&_it1->data);
                if (it1->active)
                {
                    for
                    (
                        auto it2 = it1->front2D;
                        it2 != nullptr;
                        it2 = it2->next
                    )
                    { func(it2); }
                }
            }
        }

        template <typename Func>
        inline static void foreachComponent(const Func&& func)
        {
            EntityManager2D::foreachEntity([&](Entity2D* entity)
            {
                for
                (
                    auto it = EntityManager2D::existingComponents.begin();
                    it != EntityManager2D::existingComponents.end();
                    ++it
                )
                {
                    auto component = EntityManager2D::components.find(std::make_pair(entity, it->first));
                    if (component != EntityManager2D::components.end() && component->second->active)
                        func(component->second);
                }
            });
        }

        template <typename... Ts, typename Func>
        inline static void foreachEntityWithAll(const Func&& func)
        {
            EntityManager2D::foreachEntity([&](Entity2D* entity)
            {
                Internal::Component2D* arr[sizeof...(Ts)] { std::is_same<Ts, RectTransform>::value ? entity->attachedRectTransform : EntityManager2D::components.find({ entity->attachedScene, entity, __typeid(Ts).qualifiedNameHash() })->second ... };

                bool pred = true;
                for (auto it = arr; it != arr + sizeof...(Ts); ++it)
                    pred = pred && (*it != nullptr);

                size_t i = 0;
                if (pred)
                    func(entity, (Ts*)arr[i++] ...);
            });
        }

        friend class Firework::Entity2D;
        friend class Firework::Internal::Component2D;
        friend class Firework::Internal::CoreEngine;
    };

    class __firework_corelib_api EntityManager final
    {
        static robin_hood::unordered_map<uint64_t, uint64_t> existingComponents;
        static robin_hood::unordered_map<std::pair<Entity*, uint64_t>, Internal::Component*, Internal::EntityComponentHash<Entity>> components;
    public:
        EntityManager() = delete;

        friend class Firework::Entity;
        friend class Firework::Internal::Component;
        friend class Firework::Internal::CoreEngine;
    };
}
