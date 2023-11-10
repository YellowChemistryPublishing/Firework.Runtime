#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <list>

#include <Objects/Object.h>
#include <Library/MemoryChunk.h>
#include <Library/MinAllocList.h>
#include <Library/Property.h>

namespace Firework
{
    class Entity2D;
    class RectTransform;
    class Debug;

    namespace Internal
    {
        class CoreEngine;
        
        class __firework_corelib_api Component2D : public Object
        {
            struct Reflection {
                uint64_t typeID;
            } reflection;

            Entity2D* attachedEntity = nullptr;
            RectTransform* attachedRectTransform = nullptr;
        protected:
            ~Component2D() override;
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

            /// @brief Retrieve the 2D entity associated with this component.
            /// @return Entity associated with this component.
            /// @note Main thread only.
            inline Entity2D* entity()
            {
                return this->attachedEntity;
            }
            /// @brief Retrieve the 2D transform component associated with the attached entity of this component.
            /// @return Transform component.
            /// @note Main thread only.
            inline RectTransform* rectTransform()
            {
                return this->attachedRectTransform;
            }

            friend class Firework::Entity2D;
            friend class Firework::RectTransform;
            friend class Firework::Internal::CoreEngine;
            friend class Firework::Debug;
        };
    }
}
