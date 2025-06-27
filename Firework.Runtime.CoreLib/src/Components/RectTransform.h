#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <module/sys.Mathematics>

#include <GL/Transform.h>
#include <Library/Property.h>

namespace Firework
{
    class Debug;
    class RectTransform;

    namespace Internal
    {
        class CoreEngine;

        /// @internal
        /// @brief Low-level API. Converts a RectTransform to a RenderTransform for rendering.
        /// @param transform RectTransform to convert.
        /// @return RenderTransform representing the same transform to supply to renderer.
        /// @note Thread-safe.
        __firework_corelib_api extern GL::RenderTransform renderTransformFromRectTransform(const RectTransform* const transform);
    } // namespace Internal

    /// @brief Describes the bounds of a rectangle, with int32_t.
    struct Rect
    {
        int32_t top, right, bottom, left;

        constexpr Rect(int32_t top, int32_t right, int32_t bottom, int32_t left) : top(top), right(right), bottom(bottom), left(left)
        { }
    };
    /// @brief Describes the bounds of a rectangle, with float.
    struct RectFloat
    {
        float top = 0.0f, right = 0.0f, bottom = 0.0f, left = 0.0f;

        constexpr RectFloat() noexcept = default;
        constexpr RectFloat(float with) noexcept : top(with), right(with), bottom(with), left(with)
        { }
        constexpr RectFloat(float top, float right, float bottom, float left) noexcept : top(top), right(right), bottom(bottom), left(left)
        { }

        constexpr bool operator==(const RectFloat&) const noexcept = default;

        constexpr RectFloat operator+(const RectFloat& other) const noexcept
        {
            return RectFloat { this->top + other.top, this->right + other.right, this->bottom + other.bottom, this->left + other.left };
        }
        constexpr RectFloat operator-(const RectFloat& other) const noexcept
        {
            return RectFloat { this->top - other.top, this->right - other.right, this->bottom - other.bottom, this->left - other.left };
        }
        constexpr RectFloat operator*(const RectFloat& other) const noexcept
        {
            return RectFloat { this->top * other.top, this->right * other.right, this->bottom * other.bottom, this->left * other.left };
        }
        constexpr RectFloat operator*(float other) const noexcept
        {
            return RectFloat { this->top * other, this->right * other, this->bottom * other, this->left * other };
        }
        constexpr RectFloat operator/(float other) const noexcept
        {
            return RectFloat { this->top * other, this->right * other, this->bottom * other, this->left * other };
        }

        constexpr RectFloat& operator+=(const RectFloat& other) noexcept
        {
            return (*this = *this + other);
        }
        constexpr RectFloat& operator*=(const RectFloat& other) noexcept
        {
            return (*this = *this * other);
        }

        constexpr float width() const noexcept
        {
            return this->right - this->left;
        }
        constexpr float height() const noexcept
        {
            return this->top - this->bottom;
        }
    };

    /// @brief The transform component of a 2D entity.
    class __firework_corelib_api RectTransform final
    {
        std::shared_ptr<class Entity> attachedEntity;

        RectFloat _rect { 10, 10, -10, -10 };
        RectFloat _anchor { 0, 0, 0, 0 };
        RectFloat _positionAnchor { 0, 0, 0, 0 };

        sysm::vector2 _position { 0, 0 };
        float _rotation = 0;
        sysm::vector2 _scale { 1, 1 };

        bool _dirty = true;

        std::shared_ptr<RectTransform> parent() const;

        void setRect(const RectFloat& value);

        /// @internal
        /// @brief Internal API. Set the position of this transform.
        /// @param value Position to set.
        /// @note Main thread only.
        void setPosition(sysm::vector2 value);
        /// @internal
        /// @brief Internal API. Set the rotation of this transform.
        /// @param value Rotation to set.
        /// @note Main thread only.
        void setRotation(float value);
        /// @internal
        /// @brief Internal API. Set the scale of this transform.
        /// @param value Scale to set.
        /// @note Main thread only.
        void setScale(sysm::vector2 value);

        /// @internal
        /// @brief Internal API. Retrieve the local position of this transform.
        /// @return Local position of this transform.
        /// @note Main thread only.
        sysm::vector2 getLocalPosition() const;
        /// @internal
        /// @brief Internal API. Set the local position of this transform.
        /// @param value Local position to set.
        /// @note Main thread only.
        void setLocalPosition(sysm::vector2 value);
        /// @internal
        /// @brief Internal API. Retrieve the local rotation of this transform.
        /// @return Local rotation of this transform.
        /// @note Main thread only.
        float getLocalRotation() const;
        /// @internal
        /// @brief Internal API. Set the local rotation of this transform.
        /// @param value Local rotation to set.
        /// @note Main thread only.
        void setLocalRotation(float value);
        /// @internal
        /// @brief Internal API. Retrieves the local scale of this transform.
        /// @return Local scale of this transform.
        /// @note Main thread only.
        sysm::vector2 getLocalScale() const;
        /// @internal
        /// @brief Internal API. Set the local scale of this transform.
        /// @param scale Local scale to set.
        /// @note Main thread only.
        void setLocalScale(sysm::vector2 scale);
    public:
        inline RectTransform() = default;

        /// @property
        /// @brief [Property] The rectangle bounds of this transform.
        /// @param value ```const Firework::RectFloat&```
        /// @return ```const Firework::RectFloat&```
        /// @note Main thread only.
        const Property<const RectFloat&, const RectFloat&> rect { [this]() -> const RectFloat& { return this->_rect; }, [this](const RectFloat& value)
        {
            this->setRect(value);
        } };
        /// @property
        /// @brief [Property] The anchor for the rectangle bounds of this transform.
        /// @param value ```const Firework::RectFloat&```
        /// @return ```const Firework::RectFloat&```
        /// @note Main thread only.
        const Property<const RectFloat&, const RectFloat&> rectAnchor { [this]() -> const RectFloat& { return this->_anchor; }, [this](const RectFloat& value)
        {
            this->_anchor = value;
        } };
        const Property<const RectFloat&, const RectFloat&> positionAnchor { [this]() -> const RectFloat& { return this->_positionAnchor; }, [this](const RectFloat& value)
        {
            this->_positionAnchor = value;
        } };

        /// @property
        /// @brief [Property] The position of this transform.
        /// @param value ```sysm::vector2```
        /// @return ```sysm::vector2```
        /// @note Main thread only.
        const Property<sysm::vector2, sysm::vector2> position { [this]() -> sysm::vector2 { return this->_position; }, [this](sysm::vector2 value)
        {
            this->setPosition(value);
        } };
        /// @property
        /// @brief [Property] The rotation of this transform in radians.
        /// @param value ```float```
        /// @return ```float```
        /// @note Main thread only.
        const Property<float, float> rotation { [this]() -> float { return this->_rotation; }, [this](float value)
        {
            this->setRotation(value);
        } };
        /// @property
        /// @brief [Property] The scale of this transform.
        /// @param value ```sysm::vector2```
        /// @return ```sysm::vector2```
        /// @note Main thread only.
        const Property<sysm::vector2, sysm::vector2> scale { [this]() -> sysm::vector2 { return this->_scale; }, [this](sysm::vector2 value)
        {
            this->setScale(value);
        } };

        /// @property
        /// @brief [Property] The local position of this transform.
        /// @param value ```sysm::vector2```
        /// @return ```sysm::vector2```
        /// @note Main thread only.
        const Property<sysm::vector2, sysm::vector2> localPosition { [this]() -> sysm::vector2 { return this->getLocalPosition(); }, [this](sysm::vector2 value)
        {
            this->setLocalPosition(value);
        } };
        /// @property
        /// @brief [Property] The local rotation of this transform in radians.
        /// @param value ```float```
        /// @return ```float```
        /// @note Main thread only.
        const Property<float, float> localRotation { [this]() -> float { return this->getLocalRotation(); }, [this](float value)
        {
            this->setLocalRotation(value);
        } };
        /// @property
        /// @brief [Property] The local scale of this transform.
        /// @param value ```sysm::vector2```
        /// @return ```sysm::vector2```
        /// @note Main thread only.
        const Property<sysm::vector2, sysm::vector2> localScale { [this]() -> sysm::vector2 { return this->getLocalScale(); }, [this](sysm::vector2 value)
        {
            this->setLocalScale(value);
        } };

        /// @property
        /// @brief [Property] Whether this ```Firework::RectTransform``` has been modified this logic frame.
        /// @return ```bool```
        /// @note Main thread only.
        inline bool dirty()
        {
            return this->_dirty;
        }
        /// @brief You **_shouldn't_** ever need to call this, but if you do, this sets the RectTransform-modified-this-frame flag to false.
        /// @return ```bool```
        /// @note Main thread only.
        inline void forceSetClean()
        {
            this->_dirty = false;
        }

        /// @brief Check whether a point is within the rectangle of this transform.
        /// @param point Point to query.
        /// @return Whether point is within the rectangle of this transform.
        /// @note Main thread only.
        bool queryPointIn(const sysm::vector2& point);

        friend class Firework::Internal::CoreEngine;
        friend class Firework::Entity;
        friend class Firework::Debug;
    };
} // namespace Firework
