#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

namespace Firework
{
    namespace Internal
    {
        class Object
        {
        public:
            inline virtual ~Object() = 0;
        };

        Object::~Object()
        {
        }
    }
}
