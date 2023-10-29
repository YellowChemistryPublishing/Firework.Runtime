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
            bool active = true;

            inline uint64_t typeIndex()
            {
                return this->reflection.typeID;
            }

            inline Entity2D* entity()
            {
                return this->attachedEntity;
            }
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
