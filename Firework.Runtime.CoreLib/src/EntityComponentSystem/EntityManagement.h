#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <robin_hood.h>
#include <typeindex>

#define FIREWORK_ENTITY_DECL_ONLY 1
#include <EntityComponentSystem/Entity.h>
#undef FIREWORK_ENTITY_DECL_ONLY
#include <EntityComponentSystem/SceneManagement.h>

namespace Firework
{
    class Scene;
    class Entity;

    namespace Internal
    {
        class Component2D;
        class CoreEngine;
    } // namespace Internal

    template <typename T>
    concept IEntity = std::same_as<T, Entity> || std::same_as<T, Entity>;

    class __firework_corelib_api Entities final
    {
        //                               v Component type.
        //                                                                          v Entity is key.
        //                                                                                   v Component instance.
        static robin_hood::unordered_map<std::type_index, robin_hood::unordered_map<Entity*, std::shared_ptr<void>>> table;
    public:
        Entities() = delete;

        inline static void forEachEntity(auto&& func)
        requires requires(Entity& entity) { func(entity); };
        template <typename... Ts>
        inline static void forEach(auto&& func)
        requires requires(Entity& entity) { func(entity, std::declval<Ts>()...); };

        friend class Firework::Entity;
    };

#if !defined(FIREWORK_ENTITY_MGMT_DECL_ONLY) || !FIREWORK_ENTITY_MGMT_DECL_ONLY
    inline void Entities::forEachEntity(auto&& func)
    requires requires(Entity& entity) { func(entity); }
    {
        for (Scene& scene : SceneManager::existingScenes)
        {
            auto recurse = [&](auto&& recurse, Entity* entity)
            {
                func(*entity);
                for (Entity* child = entity->_childrenFront; child; child = child->next) recurse(recurse, child);
            };
            for (Entity* entity = scene.front; entity; entity = entity->next) recurse(recurse, entity);
        }
    }
    template <typename... Ts>
    inline void Entities::forEach(auto&& func)
    requires requires(Entity& entity) { func(entity, std::declval<Ts>()...); }
    {
        if constexpr (sizeof...(Ts) == 0u)
            Entities::forEachEntity(func);
        else
        {
            const auto componentTable[] { Entities::table.find(std::type_index(typeid(Ts)))... };
            for (auto& query : componentTable)
                if (query == Entities::table.end()) [[unlikely]]
                    return;

            Entities::forEachEntity([&](Entity& entity)
            {
                const decltype(componentTable->find(nullptr)) components[sizeof...(Ts)];
                for (size_t i = 0; i < sizeof...(Ts); i++)
                {
                    components[i] = componentTable[i].find(&entity);
                    if (components[i] == componentTable[i].end())
                        return;
                }

                size_t i = 0;
                func(entity, (Ts*)components[i++]->second...);
            });
        }
    }
#endif
} // namespace Firework
