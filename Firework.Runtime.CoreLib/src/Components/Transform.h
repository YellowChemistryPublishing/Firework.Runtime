#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <Mathematics.h>
#include <Objects/Component.h>
#include <GL/Transform.h>
#include <Library/Property.h>

namespace Firework
{
    namespace Internal
    {
        __firework_corelib_api extern GL::RenderTransform renderTransformFromTransform(Transform* const transform);
    }

    class __firework_corelib_api Transform final : public Internal::Component
    {
        Mathematics::Vector3 _position { 0.0f, 0.0f, 0.0f };
        Mathematics::Quaternion _rotation { 0.0f, 0.0f, 0.0f, 1.0f };
        Mathematics::Vector3 _scale { 1.0f, 1.0f, 1.0f };

        void setPosition(Mathematics::Vector3 value);
        void setRotation(Mathematics::Quaternion value);
        void setScale(Mathematics::Vector3 value);

        Mathematics::Vector3 getLocalPosition() const;
        void setLocalPosition(Mathematics::Vector3 value);
        Mathematics::Quaternion getLocalRotation() const;
        void setLocalRotation(Mathematics::Quaternion value);
        Mathematics::Vector3 getLocalScale() const;
        void setLocalScale(Mathematics::Vector3 scale);
    public:
        const Property<Mathematics::Vector3, Mathematics::Vector3> position
        {{
            [this]() -> Mathematics::Vector3 { return this->_position; },
            [this](Mathematics::Vector3 value) { this->setPosition(value); }
        }};
        const Property<Mathematics::Quaternion, Mathematics::Quaternion> rotation
        {{
            [this]() -> Mathematics::Quaternion { return this->_rotation; },
            [this](Mathematics::Quaternion value) { this->setRotation(value); }
        }};
        const Property<Mathematics::Vector3, Mathematics::Vector3> scale
        {{
            [this]() -> Mathematics::Vector3 { return this->_scale; },
            [this](Mathematics::Vector3 value) { this->setScale(value); }
        }};
        
        const Property<Mathematics::Vector3, Mathematics::Vector3> localPosition
        {{
            [this]() -> Mathematics::Vector3 { return this->getLocalPosition(); },
            [this](Mathematics::Vector3 value) { this->setLocalPosition(value); }
        }};
        const Property<Mathematics::Quaternion, Mathematics::Quaternion> localRotation
        {{
            [this]() -> Mathematics::Quaternion { return this->getLocalRotation(); },
            [this](Mathematics::Quaternion value) { this->setLocalRotation(value); }
        }};
        const Property<Mathematics::Vector3, Mathematics::Vector3> localScale
        {{
            [this]() -> Mathematics::Vector3 { return this->getLocalScale(); },
            [this](Mathematics::Vector3 value) { this->setLocalScale(value); }
        }};

        friend __firework_corelib_api GL::RenderTransform Firework::Internal::renderTransformFromTransform(Transform* const transform);
    };
}