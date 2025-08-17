#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <iostream>
#include <robin_hood.h>
#include <typeindex>

#include <EntityComponentSystem/Entity.h>
#include <EntityComponentSystem/EntityManagement.inc>

namespace Firework
{
    inline EntityIterator Entities::begin()
    {
        return EntityIterator(Entities::front);
    }
    inline EntityIterator Entities::end()
    {
        return EntityIterator();
    }
    inline EntityRange Entities::range()
    {
        return EntityRange(Entities::front);
    }

    inline void Entities::forEachEntity(auto&& func)
    requires requires(Entity& entity) { func(entity); }
    {
        auto recurse = [&](auto&& recurse, Entity& entity) -> void
        {
            func(entity);
            for (Entity& child : entity.children()) recurse(recurse, child);
        };
        for (Entity& entity : Entities::range()) recurse(recurse, entity);
    }
    inline void Entities::forEachEntityReversed(auto&& func)
    requires requires(Entity& entity) { func(entity); }
    {
        auto recurse = [&](auto&& recurse, Entity& entity) -> void
        {
            func(entity);
            for (std::shared_ptr<Entity> child = entity._childrenBack; child; child = child->prev) recurse(recurse, *child);
        };
        for (std::shared_ptr<Entity> entity = Entities::back; entity; entity = entity->prev) recurse(recurse, *entity);
    }
    template <typename... Ts>
    inline void Entities::forEach(auto&& func)
    requires requires(Entity& entity, Ts&... components) { func(entity, components...); }
    {
        const auto invokeForEach = [&]<size_t... Is>(std::index_sequence<Is...>)
        {
            if constexpr (sizeof...(Ts) == 0u)
                Entities::forEachEntity(func);
            else
            {
                const decltype(Entities::table)::iterator componentTable[] { Entities::table.find(std::type_index(typeid(Ts)))... };
                for (auto& query : componentTable)
                    if (query == Entities::table.end()) [[unlikely]]
                        return;

                Entities::forEachEntity([&](Entity& entity)
                {
                    decltype((**componentTable).second.find(nullptr)) components[sizeof...(Ts)];
                    for (size_t i = 0; i < sizeof...(Ts); i++)
                    {
                        components[i] = componentTable[i]->second.find(&entity);
                        if (components[i] == componentTable[i]->second.end())
                            return;
                    }

                    func(entity, *std::static_pointer_cast<Ts>(components[Is]->second)...);
                });
            }
        };
        invokeForEach(std::index_sequence_for<Ts...>());
    }
} // namespace Firework
