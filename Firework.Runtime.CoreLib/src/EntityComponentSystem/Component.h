#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <list>

#include <EntityComponentSystem/Object.h>
#include <Library/Property.h>

namespace Firework
{
    class Entity2D;
    class RectTransform;
    class Debug;

    namespace Internal
    {
        class CoreEngine;

        class __firework_corelib_api Component : public Object
        {
        protected:
            ~Component() override;
        public:
            /// @brief Whether this component is active.
            /// @warning Do we use this?
            /// @note Main thread only.
            bool active = true;
        };
    } // namespace Internal
} // namespace Firework
