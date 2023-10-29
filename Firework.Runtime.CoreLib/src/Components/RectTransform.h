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

        __firework_corelib_api extern GL::RenderTransform renderTransformFromRectTransform(RectTransform* const transform);
    }

    struct Rect
    {
        int32_t top, right, bottom, left;

        constexpr Rect(int32_t top, int32_t right, int32_t bottom, int32_t left) :
        top(top), right(right), bottom(bottom), left(left)
        {
        }
    };
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

    class __firework_corelib_api RectTransform final : public Internal::Component2D
    {
        ~RectTransform() override = default;

        RectFloat _rect { 10, 10, -10, -10 };
        RectFloat _anchor { 0, 0, 0, 0 };

        Mathematics::Vector2 _position { 0, 0 };
        float _rotation = 0;
        Mathematics::Vector2 _scale { 1, 1 };

        void setPosition(Mathematics::Vector2 value);
        void setRotation(float value);
        void setScale(Mathematics::Vector2 value);

        Mathematics::Vector2 getLocalPosition() const;
        void setLocalPosition(Mathematics::Vector2 value);
        float getLocalRotation() const;
        void setLocalRotation(float value);
        Mathematics::Vector2 getLocalScale() const;
        void setLocalScale(Mathematics::Vector2 scale);
    public:
        const Property<const RectFloat&, const RectFloat&> rect
        {{
            [this]() -> const RectFloat& { return this->_rect; },
            [this](const RectFloat& value) { this->_rect = value; }
        }};
        const Property<const RectFloat&, const RectFloat&> rectAnchor
        {{
            [this]() -> const RectFloat& { return this->_anchor; },
            [this](const RectFloat& value) { this->_anchor = value; }
        }};

        const Property<Mathematics::Vector2, Mathematics::Vector2> position
        {{
            [this]() -> Mathematics::Vector2 { return this->_position; },
            [this](Mathematics::Vector2 value) { this->setPosition(value); }
        }};
        const Property<float, float> rotation
        {{
            [this]() -> float { return this->_rotation; },
            [this](float value) { this->setRotation(value); }
        }};
        const Property<Mathematics::Vector2, Mathematics::Vector2> scale
        {{
            [this]() -> Mathematics::Vector2 { return this->_scale; },
            [this](Mathematics::Vector2 value) { this->setScale(value); }
        }};
        
        const Property<Mathematics::Vector2, Mathematics::Vector2> localPosition
        {{
            [this]() -> Mathematics::Vector2 { return this->getLocalPosition(); },
            [this](Mathematics::Vector2 value) { this->setLocalPosition(value); }
        }};
        const Property<float, float> localRotation
        {{
            [this]() -> float { return this->getLocalRotation(); },
            [this](float value) { this->setLocalRotation(value); }
        }};
        const Property<Mathematics::Vector2, Mathematics::Vector2> localScale
        {{
            [this]() -> Mathematics::Vector2 { return this->getLocalScale(); },
            [this](Mathematics::Vector2 value) { this->setLocalScale(value); }
        }};

        bool queryPointIn(const Mathematics::Vector2& point);

        friend class Firework::Entity2D;
        friend class Firework::Debug;
    };
}
