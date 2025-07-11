#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <robin_hood.h>
#include <typeindex>

namespace Firework
{
    struct EntityIterator;
    struct EntityRange;
    class Entity;

    namespace Internal
    {
        class Component2D;
        class CoreEngine;
    } // namespace Internal

    class _fw_core_api Entities final
    {
        //                               v Component type.
        //                                                                          v Entity is key.
        //                                                                                   v Component instance.
        static robin_hood::unordered_map<std::type_index, robin_hood::unordered_map<Entity*, std::shared_ptr<void>>> table;

        static std::shared_ptr<Entity> front;
        static std::shared_ptr<Entity> back;
    public:
        Entities() = delete;

        inline static EntityIterator begin();
        inline static EntityIterator end();
        inline static EntityRange range();

        inline static void forEachEntity(auto&& func)
        requires requires(Entity& entity) { func(entity); };
        template <typename... Ts>
        inline static void forEach(auto&& func)
        requires requires(Entity& entity) { func(entity, std::declval<Ts>()...); };

        friend class Firework::Internal::CoreEngine;
        friend class Firework::Entity;
    };
} // namespace Firework
