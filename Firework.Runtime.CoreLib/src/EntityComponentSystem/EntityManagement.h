#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"
#include "Objects/Object.h"

#include <initializer_list>
#include <list>
#include <map>
#include <robin_hood.h>
#include <tuple>
#include <typeindex>

#include <Objects/Entity.inc>
#include <Objects/Entity2D.h>

namespace Firework
{
    class Scene;

    class Entity2D;
    class Entity;

    namespace Internal
    {
        class Component2D;
        class CoreEngine;

        /// @internal
        /// @brief Internal API. Hashes an entity-component pair.
        /// @tparam EntityType Either ```Firework::Entity2D``` or ```Firework::Entity```.
        /// @note Thread-safe.
        template <typename EntityType>
        requires std::same_as<EntityType, Entity2D> || std::same_as<EntityType, Entity>
        struct EntityComponentHash // Specialization appears before first use.
        {
            size_t operator()(const std::pair<EntityType*, uint64_t>& value) const
            {
                size_t a = std::hash<EntityType*>()(value.first);
                size_t b = std::hash<uint64_t>()(value.second);
                return (a + b) / 2 * (a + b + 1) + b; // Cantor pairing function. Good enough.
            }
        };
    } // namespace Internal

    template <typename T>
    concept IEntity = std::same_as<T, Entity2D> || std::same_as<T, Entity>;

    class __firework_corelib_api Entities final
    {
        static robin_hood::unordered_set<Internal::Object*> entities;
        //                               v Component type.
        //                                                                          v Entity is key.
        //                                                                                 v Component instance.
        static robin_hood::unordered_map<std::type_index, robin_hood::unordered_map<void*, void*>> table;
    public:
        Entities() = delete;

        template <IEntity Entity, typename... Ts>
        inline static void forEach(auto&& func)
        requires requires(Entity& entity) { func(entity, std::declval<Ts>()...); }
        {
            if constexpr (sizeof...(Ts) == 0u)
            {
                for (Internal::Object* entity : Entities::entities)
                {
                    if (Entity* requestedEntity = dynamic_cast<Entity*>(entity))
                        func(*requestedEntity);
                }
            }
            else
            {
                const decltype(Entities::table.find(nullptr)) componentTable[sizeof...(Ts)];
                for (auto query : componentTable)
                    if (query == Entities::table.end()) [[unlikely]]
                        return;

                for (Internal::Object* entity : Entities::entities)
                {
                    if (Entity* requestedEntity = dynamic_cast<Entity*>(entity))
                    {
                        
                        func(*requestedEntity);
                    }
                }
            }
        }
    };
} // namespace Firework
