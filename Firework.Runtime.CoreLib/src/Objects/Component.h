#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <cstdint>
#include <list>

#include <Objects/Object.h>

namespace Firework
{
    class Entity;
    class Transform;
    class Debug;

    namespace Internal
    {
        class CoreEngine;
        
        class __firework_corelib_api Component : public Object
        {
            struct Reflection {
                uint64_t typeID;
            } reflection;

            Entity* attachedEntity = nullptr;
            Transform* attachedTransform = nullptr;
        protected:
            inline ~Component() override = default;
        public:
            /// @brief Whether this component is active.
            /// @warning Do we use this?
            /// @note Main thread only.
            bool active = true;

            /// @brief The type hash of this component.
            /// @return Value of __typeid(T).qualifiedNameHash(), where T is the type of the component.
            /// @note Main thread only.
            inline uint64_t typeIndex()
            {
                return this->reflection.typeID;
            }

            /// @brief Retrieve the 3D entity associated with this component.
            /// @return Entity associated with this component.
            /// @note Main thread only.
            inline Entity* entity()
            {
                return this->attachedEntity;
            }
            /// @brief Retrieve the 3D transform component associated with the attached entity of this component.
            /// @return Transform component.
            /// @note Main thread only.
            inline Transform* transform()
            {
                return this->attachedTransform;
            }

            friend class Firework::Entity;
            friend class Firework::Transform;
            friend class Firework::Internal::CoreEngine;
            friend class Firework::Debug;
        };
    }
}
