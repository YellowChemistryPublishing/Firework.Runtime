#pragma once

#include "Firework.Components.Core3D.Exports.h"

#include <Objects/Component.h>

namespace Firework
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;
    }

    class __firework_componentcore3d_api Camera final : public Internal::Component
    {
        static std::list<Camera*> all;
        static Camera* mainCamera;

        std::list<Camera*>::iterator it;

        void project();
    public:
        float yFov = 60.0f, near = 0.0f, far = 119.428223f;

        Camera();
        ~Camera() override;

        inline static Camera* active()
        {
            return Camera::mainCamera;
        }

        friend struct Firework::Internal::ComponentCoreStaticInit;
    };
}