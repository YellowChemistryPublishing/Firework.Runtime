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
        /// @internal
        /// @brief Low-level API. Converts a 3D Transform to a RenderTransform for rendering.
        /// @param transform Transform to convert.
        /// @return RenderTransform representing the same transform to supply to renderer.
        /// @note Thread-safe.
        __firework_corelib_api extern GL::RenderTransform renderTransformFromTransform(Transform* const transform);
    }

    /// @brief The transform component of a 3D entity.
    class __firework_corelib_api Transform final : public Internal::Component
    {
        Mathematics::Vector3 _position { 0.0f, 0.0f, 0.0f };
        Mathematics::Quaternion _rotation { 0.0f, 0.0f, 0.0f, 1.0f };
        Mathematics::Vector3 _scale { 1.0f, 1.0f, 1.0f };

        /// @internal
        /// @brief Internal API. Set the position of this transform.
        /// @param value Position to set.
        /// @note Main thread only.
        void setPosition(Mathematics::Vector3 value);
        /// @internal
        /// @brief Internal API. Set the rotation of this transform.
        /// @param value Rotation to set.
        /// @note Main thread only.
        void setRotation(Mathematics::Quaternion value);
        /// @internal
        /// @brief Internal API. Set the scale of this transform.
        /// @param value Scale to set.
        /// @note Main thread only.
        void setScale(Mathematics::Vector3 value);

        /// @internal
        /// @brief Internal API. Retrieve the local position of this transform.
        /// @return Local position of this transform.
        /// @note Main thread only.
        Mathematics::Vector3 getLocalPosition() const;
        /// @internal
        /// @brief Internal API. Set the local position of this transform.
        /// @param value Local position to set.
        /// @note Main thread only.
        void setLocalPosition(Mathematics::Vector3 value);
        /// @internal
        /// @brief Internal API. Retrieve the local rotation of this transform.
        /// @return Local rotation of this transform.
        /// @note Main thread only.
        Mathematics::Quaternion getLocalRotation() const;
        /// @internal
        /// @brief Internal API. Set the local rotation of this transform.
        /// @param value Local rotation to set.
        /// @note Main thread only.
        void setLocalRotation(Mathematics::Quaternion value);
        /// @internal
        /// @brief Internal API. Retrieves the local scale of this transform.
        /// @return Local scale of this transform.
        /// @note Main thread only.
        Mathematics::Vector3 getLocalScale() const;
        /// @internal
        /// @brief Internal API. Set the local scale of this transform.
        /// @param scale Local scale to set.
        /// @note Main thread only.
        void setLocalScale(Mathematics::Vector3 scale);
    public:
        /// @property
        /// @brief [Property] The position of this transform.
        /// @param value ```Firework::Mathematics::Vector3```
        /// @return ```Firework::Mathematics::Vector3```
        /// @note Main thread only.
        const Property<Mathematics::Vector3, Mathematics::Vector3> position
        {{
            [this]() -> Mathematics::Vector3 { return this->_position; },
            [this](Mathematics::Vector3 value) { this->setPosition(value); }
        }};
        /// @property
        /// @brief [Property] The rotation of this transform in radians.
        /// @param value ```Firework::Mathematics::Quaternion```
        /// @return ```Firework::Mathematics::Quaternion```
        /// @see Firework::Mathematics::Quaternion::fromEuler when using euler angles for rotations.
        /// @note Main thread only.
        const Property<Mathematics::Quaternion, Mathematics::Quaternion> rotation
        {{
            [this]() -> Mathematics::Quaternion { return this->_rotation; },
            [this](Mathematics::Quaternion value) { this->setRotation(value); }
        }};
        /// @property
        /// @brief [Property] The scale of this transform.
        /// @param value ```Firework::Mathematics::Vector3```
        /// @return ```Firework::Mathematics::Vector3```
        /// @note Main thread only.
        const Property<Mathematics::Vector3, Mathematics::Vector3> scale
        {{
            [this]() -> Mathematics::Vector3 { return this->_scale; },
            [this](Mathematics::Vector3 value) { this->setScale(value); }
        }};
        
        /// @property
        /// @brief [Property] The local position of this transform.
        /// @param value ```Firework::Mathematics::Vector3```
        /// @return ```Firework::Mathematics::Vector3```
        /// @note Main thread only.
        const Property<Mathematics::Vector3, Mathematics::Vector3> localPosition
        {{
            [this]() -> Mathematics::Vector3 { return this->getLocalPosition(); },
            [this](Mathematics::Vector3 value) { this->setLocalPosition(value); }
        }};
        /// @property
        /// @brief [Property] The local rotation of this transform in radians.
        /// @param value ```Firework::Mathematics::Quaternion```
        /// @return ```Firework::Mathematics::Quaternion```
        /// @see Firework::Mathematics::Quaternion::fromEuler when using euler angles for rotations.
        /// @note Main thread only.
        const Property<Mathematics::Quaternion, Mathematics::Quaternion> localRotation
        {{
            [this]() -> Mathematics::Quaternion { return this->getLocalRotation(); },
            [this](Mathematics::Quaternion value) { this->setLocalRotation(value); }
        }};
        /// @property
        /// @brief [Property] The local scale of this transform.
        /// @param value ```Firework::Mathematics::Vector3```
        /// @return ```Firework::Mathematics::Vector3```
        /// @note Main thread only.
        const Property<Mathematics::Vector3, Mathematics::Vector3> localScale
        {{
            [this]() -> Mathematics::Vector3 { return this->getLocalScale(); },
            [this](Mathematics::Vector3 value) { this->setLocalScale(value); }
        }};

        friend __firework_corelib_api GL::RenderTransform Firework::Internal::renderTransformFromTransform(Transform* const transform);
    };
}