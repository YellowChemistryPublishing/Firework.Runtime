#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <Mathematics.h>
#include <GL/Transform.h>
#include <Objects/Component2D.h>
#include <Library/Property.h>

namespace Firework
{
    class Debug;

    namespace Internal
    {
        class CoreEngine;

        /// @internal
        /// @brief Low-level API [Internal]. Converts a RectTransform to a RenderTransform for rendering.
        /// @param transform RectTransform to convert.
        /// @return RenderTransform representing the same transform to supply to renderer.
        /// @note Thread-safe.
        __firework_corelib_api extern GL::RenderTransform renderTransformFromRectTransform(RectTransform* const transform);
    }

    /// @brief Describes the bounds of a rectangle, with int32_t.
    struct Rect
    {
        int32_t top, right, bottom, left;

        constexpr Rect(int32_t top, int32_t right, int32_t bottom, int32_t left) :
        top(top), right(right), bottom(bottom), left(left)
        {
        }
    };
    /// @brief Describes the bounds of a rectangle, with float.
    struct RectFloat
    {
        float top = 0.0f, right = 0.0f, bottom = 0.0f, left = 0.0f;

        constexpr RectFloat() noexcept = default;
        constexpr RectFloat(float top, float right, float bottom, float left) noexcept :
        top(top), right(right), bottom(bottom), left(left)
        { }
 
        constexpr bool operator==(const RectFloat&) const noexcept = default;
        
        constexpr RectFloat operator+(const RectFloat& other) const noexcept
        {
            return RectFloat
            {
                this->top + other.top,
                this->right + other.right,
                this->bottom + other.bottom,
                this->left + other.left
            };
        }
    };

    /// @brief The transform component of a 2D entity.
    class __firework_corelib_api RectTransform final : public Internal::Component2D
    {
        ~RectTransform() override = default;

        RectFloat _rect { 10, 10, -10, -10 };
        RectFloat _anchor { 0, 0, 0, 0 };

        Mathematics::Vector2 _position { 0, 0 };
        float _rotation = 0;
        Mathematics::Vector2 _scale { 1, 1 };

        /// @internal
        /// @brief Internal API [Internal]. Set the position of this transform.
        /// @param value Position to set.
        void setPosition(Mathematics::Vector2 value);
        /// @internal
        /// @brief Internal API [Internal]. Set the rotation of this transform.
        /// @param value Rotation to set.
        void setRotation(float value);
        /// @internal
        /// @brief Internal API [Internal]. Set the scale of this transform.
        /// @param value Scale to set.
        void setScale(Mathematics::Vector2 value);

        /// @internal
        /// @brief Internal API [Internal]. Retrieve the local position of this transform.
        /// @return Local position of this transform.
        Mathematics::Vector2 getLocalPosition() const;
        /// @internal
        /// @brief Internal API [Internal]. Set the local position of this transform.
        /// @param value Local position to set.
        void setLocalPosition(Mathematics::Vector2 value);
        /// @internal
        /// @brief Internal API [Internal]. Retrieve the local rotation of this transform.
        /// @return Local rotation of this transform.
        float getLocalRotation() const;
        /// @internal
        /// @brief Internal API [Internal]. Set the local rotation of this transform.
        /// @param value Local rotation to set.
        void setLocalRotation(float value);
        /// @internal
        /// @brief Internal API [Internal]. Retrieves the local scale of this transform.
        /// @return Local scale of this transform.
        Mathematics::Vector2 getLocalScale() const;
        /// @internal
        /// @brief Internal API [Internal]. Set the local scale of this transform.
        /// @param scale Local scale to set.
        void setLocalScale(Mathematics::Vector2 scale);
    public:
        /// @property
        /// @brief [Property] The rectangle bounds of this transform.
        /// @param value ```const Firework::RectFloat&```
        /// @return ```const Firework::RectFloat&```
        const Property<const RectFloat&, const RectFloat&> rect
        {{
            [this]() -> const RectFloat& { return this->_rect; },
            [this](const RectFloat& value) { this->_rect = value; }
        }};
        /// @property
        /// @brief [Property] The anchor for the rectangle bounds of this transform.
        /// @param value ```const Firework::RectFloat&```
        /// @return ```const Firework::RectFloat&```
        const Property<const RectFloat&, const RectFloat&> rectAnchor
        {{
            [this]() -> const RectFloat& { return this->_anchor; },
            [this](const RectFloat& value) { this->_anchor = value; }
        }};

        /// @property
        /// @brief [Property] The position of this transform.
        /// @param value ```Mathematics::Vector2```
        /// @return ```Mathematics::Vector2```
        const Property<Mathematics::Vector2, Mathematics::Vector2> position
        {{
            [this]() -> Mathematics::Vector2 { return this->_position; },
            [this](Mathematics::Vector2 value) { this->setPosition(value); }
        }};
        /// @property
        /// @brief [Property] The rotation of this transform in radians.
        /// @param value ```float```
        /// @return ```float```
        const Property<float, float> rotation
        {{
            [this]() -> float { return this->_rotation; },
            [this](float value) { this->setRotation(value); }
        }};
        /// @property
        /// @brief [Property] The scale of this transform.
        /// @param value ```Mathematics::Vector2```
        /// @return ```Mathematics::Vector2```
        const Property<Mathematics::Vector2, Mathematics::Vector2> scale
        {{
            [this]() -> Mathematics::Vector2 { return this->_scale; },
            [this](Mathematics::Vector2 value) { this->setScale(value); }
        }};
        
        /// @property
        /// @brief [Property] The local position of this transform.
        /// @param value ```Mathematics::Vector2```
        /// @return ```Mathematics::Vector2```
        const Property<Mathematics::Vector2, Mathematics::Vector2> localPosition
        {{
            [this]() -> Mathematics::Vector2 { return this->getLocalPosition(); },
            [this](Mathematics::Vector2 value) { this->setLocalPosition(value); }
        }};
        /// @property
        /// @brief [Property] The local rotation of this transform in radians.
        /// @param value ```float```
        /// @return ```float```
        const Property<float, float> localRotation
        {{
            [this]() -> float { return this->getLocalRotation(); },
            [this](float value) { this->setLocalRotation(value); }
        }};
        /// @property
        /// @brief [Property] The local scale of this transform.
        /// @param value ```Mathematics::Vector2```
        /// @return ```Mathematics::Vector2```
        const Property<Mathematics::Vector2, Mathematics::Vector2> localScale
        {{
            [this]() -> Mathematics::Vector2 { return this->getLocalScale(); },
            [this](Mathematics::Vector2 value) { this->setLocalScale(value); }
        }};

        /// @brief Check whether a point is within the rectangle of this transform.
        /// @param point Point to query.
        /// @return Whether point is within the rectangle of this transform.
        bool queryPointIn(const Mathematics::Vector2& point);

        friend class Firework::Entity2D;
        friend class Firework::Debug;
    };
}
