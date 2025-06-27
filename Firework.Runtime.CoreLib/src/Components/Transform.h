#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <module/sys.Mathematics>

#include <EntityComponentSystem/Component.h>
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
    } // namespace Internal

    /// @brief The transform component of a 3D entity.
    class __firework_corelib_api Transform final : public Internal::Component
    {
        sysm::vector3 _position { 0.0f, 0.0f, 0.0f };
        sysm::quaternion _rotation { 0.0f, 0.0f, 0.0f, 1.0f };
        sysm::vector3 _scale { 1.0f, 1.0f, 1.0f };

        /// @internal
        /// @brief Internal API. Set the position of this transform.
        /// @param value Position to set.
        /// @note Main thread only.
        void setPosition(sysm::vector3 value);
        /// @internal
        /// @brief Internal API. Set the rotation of this transform.
        /// @param value Rotation to set.
        /// @note Main thread only.
        void setRotation(sysm::quaternion value);
        /// @internal
        /// @brief Internal API. Set the scale of this transform.
        /// @param value Scale to set.
        /// @note Main thread only.
        void setScale(sysm::vector3 value);

        /// @internal
        /// @brief Internal API. Retrieve the local position of this transform.
        /// @return Local position of this transform.
        /// @note Main thread only.
        sysm::vector3 getLocalPosition() const;
        /// @internal
        /// @brief Internal API. Set the local position of this transform.
        /// @param value Local position to set.
        /// @note Main thread only.
        void setLocalPosition(sysm::vector3 value);
        /// @internal
        /// @brief Internal API. Retrieve the local rotation of this transform.
        /// @return Local rotation of this transform.
        /// @note Main thread only.
        sysm::quaternion getLocalRotation() const;
        /// @internal
        /// @brief Internal API. Set the local rotation of this transform.
        /// @param value Local rotation to set.
        /// @note Main thread only.
        void setLocalRotation(sysm::quaternion value);
        /// @internal
        /// @brief Internal API. Retrieves the local scale of this transform.
        /// @return Local scale of this transform.
        /// @note Main thread only.
        sysm::vector3 getLocalScale() const;
        /// @internal
        /// @brief Internal API. Set the local scale of this transform.
        /// @param scale Local scale to set.
        /// @note Main thread only.
        void setLocalScale(sysm::vector3 scale);
    public:
        /// @property
        /// @brief [Property] The position of this transform.
        /// @param value ```sysm::vector3```
        /// @return ```sysm::vector3```
        /// @note Main thread only.
        const Property<sysm::vector3, sysm::vector3> position { [this]() -> sysm::vector3 { return this->_position; }, [this](sysm::vector3 value)
        {
            this->setPosition(value);
        } };
        /// @property
        /// @brief [Property] The rotation of this transform in radians.
        /// @param value ```sysm::quaternion```
        /// @return ```sysm::quaternion```
        /// @see sysm::quaternion::fromEuler when using euler angles for rotations.
        /// @note Main thread only.
        const Property<sysm::quaternion, sysm::quaternion> rotation { [this]() -> sysm::quaternion { return this->_rotation; }, [this](sysm::quaternion value)
        {
            this->setRotation(value);
        } };
        /// @property
        /// @brief [Property] The scale of this transform.
        /// @param value ```sysm::vector3```
        /// @return ```sysm::vector3```
        /// @note Main thread only.
        const Property<sysm::vector3, sysm::vector3> scale { [this]() -> sysm::vector3 { return this->_scale; }, [this](sysm::vector3 value)
        {
            this->setScale(value);
        } };

        /// @property
        /// @brief [Property] The local position of this transform.
        /// @param value ```sysm::vector3```
        /// @return ```sysm::vector3```
        /// @note Main thread only.
        const Property<sysm::vector3, sysm::vector3> localPosition { [this]() -> sysm::vector3 { return this->getLocalPosition(); }, [this](sysm::vector3 value)
        {
            this->setLocalPosition(value);
        } };
        /// @property
        /// @brief [Property] The local rotation of this transform in radians.
        /// @param value ```sysm::quaternion```
        /// @return ```sysm::quaternion```
        /// @see sysm::quaternion::fromEuler when using euler angles for rotations.
        /// @note Main thread only.
        const Property<sysm::quaternion, sysm::quaternion> localRotation { [this]() -> sysm::quaternion { return this->getLocalRotation(); }, [this](sysm::quaternion value)
        {
            this->setLocalRotation(value);
        } };
        /// @property
        /// @brief [Property] The local scale of this transform.
        /// @param value ```sysm::vector3```
        /// @return ```sysm::vector3```
        /// @note Main thread only.
        const Property<sysm::vector3, sysm::vector3> localScale { [this]() -> sysm::vector3 { return this->getLocalScale(); }, [this](sysm::vector3 value)
        {
            this->setLocalScale(value);
        } };

        friend __firework_corelib_api GL::RenderTransform Firework::Internal::renderTransformFromTransform(Transform* const transform);
    };
} // namespace Firework