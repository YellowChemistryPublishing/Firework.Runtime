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
            bool active = true;

            inline uint64_t typeIndex()
            {
                return this->reflection.typeID;
            }

            inline Entity* entity()
            {
                return this->attachedEntity;
            }
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
