#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <robin_hood.h>
#include <typeindex>

#include <Objects/Entity.h>
#include <Objects/Object.h>

namespace Firework
{
    class Scene;

    class Entity;

    namespace Internal
    {
        class Component2D;
        class CoreEngine;

        /// @internal
        /// @brief Internal API. Hashes an entity-component pair.
        /// @tparam EntityType Either ```Firework::Entity``` or ```Firework::Entity```.
        /// @note Thread-safe.
        template <typename EntityType>
        requires std::same_as<EntityType, Entity> || std::same_as<EntityType, Entity>
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
    concept IEntity = std::same_as<T, Entity> || std::same_as<T, Entity>;

    class __firework_corelib_api Entities final
    {
        //                               v Component type.
        //                                                                          v Entity is key.
        //                                                                                   v Component instance.
        static robin_hood::unordered_map<std::type_index, robin_hood::unordered_map<Entity*, std::shared_ptr<void>>> table;
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
                const decltype(Entities::table.find(nullptr)) componentTable[] { Entities::table.find(std::type_index(typeid(Ts)))... };
                for (auto& query : componentTable)
                    if (query == Entities::table.end()) [[unlikely]]
                        return;

                for (Internal::Object* entity : Entities::entities)
                {
                    if (Entity* requestedEntity = dynamic_cast<Entity*>(entity))
                    {
                        const decltype(componentTable->find(nullptr)) components[sizeof...(Ts)];
                        for (size_t i = 0; i < sizeof...(Ts); i++)
                        {
                            components[i] = componentTable[i].find(requestedEntity);
                            if (components[i] == componentTable[i].end())
                                goto Continue;
                        }

                        size_t i = 0;
                        func(*requestedEntity, (Ts*)components[i++]->second...);
                    }
                Continue:;
                }
            }
        }

        friend class Firework::Entity;
    };
} // namespace Firework
