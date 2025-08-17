#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <robin_hood.h>
#include <typeindex>

namespace Firework::Internal
{
    class Component2D;
    class CoreEngine;
} // namespace Firework::Internal

namespace Firework
{
    struct EntityIterator;
    struct EntityRange;
    class Entity;

    class Entities final
    {
        //                               v Component type.
        //                                                                          v Entity is key.
        //                                                                                   v Component instance.
        static _fw_core_api robin_hood::unordered_map<std::type_index, robin_hood::unordered_map<Entity*, std::shared_ptr<void>>> table;

        static _fw_core_api std::shared_ptr<Entity> front;
        static _fw_core_api std::shared_ptr<Entity> back;
    public:
        Entities() = delete;

        static EntityIterator begin();
        static EntityIterator end();
        static EntityRange range();

        inline static void forEachEntity(auto&& func)
        requires requires(Entity& entity) { func(entity); };
        inline static void forEachEntityReversed(auto&& func)
        requires requires(Entity& entity) { func(entity); };
        template <typename... Ts>
        inline static void forEach(auto&& func)
        requires requires(Entity& entity, Ts&... components) { func(entity, components...); };

        friend class Firework::Internal::CoreEngine;
        friend class Firework::Entity;
    };
} // namespace Firework
