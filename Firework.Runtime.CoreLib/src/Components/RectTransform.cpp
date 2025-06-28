#include "RectTransform.h"

#include <cmath>
#include <module/sys.Mathematics>
#include <numbers>

#include <EntityComponentSystem/Entity.h>
#include <GL/Renderer.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;

inline static void rotatePointAround(sysm::vector2& point, const sysm::vector2& rotateAround, float angle)
{
    float s = std::sinf(-angle * std::numbers::pi_v<float> / 180.0f);
    float c = std::cosf(-angle * std::numbers::pi_v<float> / 180.0f);

    float x = point.x - rotateAround.x;
    float y = point.y - rotateAround.y;

    point.x = x * c - y * s + rotateAround.x;
    point.y = x * s + y * c + rotateAround.y;
};
inline static void rotatePointAroundOrigin(sysm::vector2& point, float angle)
{
    return rotatePointAround(point, sysm::vector2::zero, angle);
};

RenderTransform Firework::Internal::renderTransformFromRectTransform(const RectTransform* const transform)
{
    RenderTransform ret;
    ret.scale({ transform->rect().width() / 2.0f * transform->scale().x, transform->rect().height() / 2.0f * transform->scale().y, 0 });
    ret.translate({ (transform->rect().right + transform->rect().left) / 2.0f, (transform->rect().top + transform->rect().bottom) / 2.0f, 0.0f });
    ret.rotate(Renderer::fromEuler({ 0, 0, -transform->rotation() }));
    ret.translate({ transform->position().x, transform->position().y, 0.0f });
    return ret;
}

void RectTransform::setRect(const RectFloat& value)
{
    this->_dirty = true;

    RectFloat delta = value - this->_rect;
    this->_rect = value;

    auto recurse = [&](auto&& recurse, Entity& entity, RectFloat delta) -> void
    {
        for (auto it = entity._childrenFront; it; it = it->next)
        {
            RectFloat localDelta = delta;
            if (std::shared_ptr<RectTransform> rectTransform = it->getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                localDelta = delta * rectTransform->_anchor;
                rectTransform->_rect += localDelta;
                if (rectTransform->_positionAnchor != RectFloat(0.0f))
                {
                    rectTransform->setLocalPosition(
                        rectTransform->getLocalPosition() +
                        sysm::vector2(delta.right, delta.top) * sysm::vector2(rectTransform->_positionAnchor.right, rectTransform->_positionAnchor.top) +
                        sysm::vector2(delta.left, delta.bottom) * sysm::vector2(rectTransform->_positionAnchor.left, rectTransform->_positionAnchor.bottom));
                }
            }
            recurse(recurse, *it, localDelta);
        }
    };
    recurse(recurse, *this->attachedEntity, delta);
}

void RectTransform::setPosition(sysm::vector2 value)
{
    this->_dirty = true;

    sysm::vector2 delta = value - this->_position;
    this->_position = value;

    auto setChildrenPositionRecursive = [&](auto&& setChildrenPositionRecursive, Entity& entity) -> void
    {
        for (auto it = entity._childrenFront; it; it = it->next)
        {
            if (std::shared_ptr<RectTransform> rectTransform = it->getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->_position += delta;
            }
            setChildrenPositionRecursive(setChildrenPositionRecursive, *it);
        }
    };
    setChildrenPositionRecursive(setChildrenPositionRecursive, *this->attachedEntity);
}
void RectTransform::setRotation(float value)
{
    this->_dirty = true;

    float delta = value - this->_rotation;
    this->_rotation = value;

    auto setChildrenRotationRecursive = [&, this](auto&& setChildrenRotationRecursive, Entity& entity) -> void
    {
        for (auto it = entity._childrenFront; it; it = it->next)
        {
            if (std::shared_ptr<RectTransform> rectTransform = it->getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->_rotation += delta;
                rotatePointAround(rectTransform->_position, this->_position, delta);
            }
            setChildrenRotationRecursive(setChildrenRotationRecursive, *it);
        }
    };
    setChildrenRotationRecursive(setChildrenRotationRecursive, *this->attachedEntity);
}
void RectTransform::setScale(sysm::vector2 value)
{
    this->_dirty = true;

    sysm::vector2 delta = value / this->_scale;
    this->_scale = value;

    auto setChildrenScaleRecursive = [&, this](auto&& setChildrenScaleRecursive, Entity& entity) -> void
    {
        for (auto it = entity._childrenFront; it; it = it->next)
        {
            if (std::shared_ptr<RectTransform> rectTransform = it->getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->_scale *= delta;
                rectTransform->_position = delta * (rectTransform->_position - this->_position) + this->_position;
            }
            setChildrenScaleRecursive(setChildrenScaleRecursive, *it);
        }
    };
    setChildrenScaleRecursive(setChildrenScaleRecursive, *this->attachedEntity);
}

std::shared_ptr<RectTransform> RectTransform::parent() const
{
    for (std::shared_ptr<Entity> parent = this->attachedEntity->_parent; parent; parent = parent->_parent)
    {
        if (std::shared_ptr<RectTransform> rectTransform = parent->getComponent<RectTransform>())
            return rectTransform;
    }
    return nullptr;
}

sysm::vector2 RectTransform::getLocalPosition() const
{
    std::shared_ptr<RectTransform> parent = this->parent();
    if (parent)
    {
        sysm::vector2 locPos = (this->_position - parent->_position) / parent->_scale;
        rotatePointAroundOrigin(locPos, -parent->_rotation);
        return locPos;
    }
    else
        return this->_position;
}
void RectTransform::setLocalPosition(sysm::vector2 value)
{
    this->_dirty = true;

    std::shared_ptr<RectTransform> parent = this->parent();
    sysm::vector2 delta;
    if (parent)
    {
        rotatePointAroundOrigin(value, parent->_rotation);
        delta = parent->_position + parent->_scale * value - this->_position;
    }
    else
        delta = value - this->_position;
    this->_position += delta;

    auto setChildrenPositionRecursive = [&](auto&& setChildrenPositionRecursive, Entity& entity) -> void
    {
        for (auto it = entity._childrenFront; it; it = it->next)
        {
            if (std::shared_ptr<RectTransform> rectTransform = it->getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->_position += delta;
            }
            setChildrenPositionRecursive(setChildrenPositionRecursive, *it);
        }
    };
    setChildrenPositionRecursive(setChildrenPositionRecursive, *this->attachedEntity);
}
float RectTransform::getLocalRotation() const
{
    std::shared_ptr<RectTransform> parent = this->parent();
    return parent ? this->_rotation - parent->_rotation : this->_rotation;
}
void RectTransform::setLocalRotation(float value)
{
    this->_dirty = true;

    std::shared_ptr<RectTransform> parent = this->parent();
    float delta;
    if (parent)
        delta = parent->_rotation + value - this->_rotation;
    else
        delta = value - this->_rotation;
    this->_rotation += delta;

    auto setChildrenRotationRecursive = [&, this](auto&& setChildrenRotationRecursive, Entity& entity) -> void
    {
        for (auto it = entity._childrenFront; it != nullptr; it = it->next)
        {
            if (std::shared_ptr<RectTransform> rectTransform = it->getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->_rotation += delta;
                rotatePointAround(rectTransform->_position, this->_position, delta);
            }
            setChildrenRotationRecursive(setChildrenRotationRecursive, *it);
        }
    };
    setChildrenRotationRecursive(setChildrenRotationRecursive, *this->attachedEntity);
}
sysm::vector2 RectTransform::getLocalScale() const
{
    std::shared_ptr<RectTransform> parent = this->parent();
    return parent ? this->_scale / parent->_scale : this->_scale;
}
void RectTransform::setLocalScale(sysm::vector2 value)
{
    this->_dirty = true;

    std::shared_ptr<RectTransform> parent = this->parent();
    sysm::vector2 delta;
    if (parent)
        delta = value * parent->_scale / this->_scale;
    else
        delta = value / this->_scale;
    this->_scale *= delta;

    auto setChildrenScaleRecursive = [&, this](auto&& setChildrenScaleRecursive, Entity& entity) -> void
    {
        for (auto it = entity._childrenFront; it != nullptr; it = it->next)
        {
            if (std::shared_ptr<RectTransform> rectTransform = it->getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->_scale *= delta;
                rectTransform->_position = delta * (rectTransform->_position - this->_position) + this->_position;
            }
            setChildrenScaleRecursive(setChildrenScaleRecursive, *it);
        }
    };
    setChildrenScaleRecursive(setChildrenScaleRecursive, *this->attachedEntity);
}

bool RectTransform::queryPointIn(const sysm::vector2& point)
{
    float rL = this->_rect.left * this->_scale.x;
    float rB = this->_rect.bottom * this->_scale.y;

    sysm::vector2 vA(this->_rect.right * this->_scale.x, rB);
    rotatePointAroundOrigin(vA, this->_rotation);
    sysm::vector2 vB(rL, rB);
    rotatePointAroundOrigin(vB, this->_rotation);
    sysm::vector2 vC(rL, this->_rect.top * this->_scale.y);
    rotatePointAroundOrigin(vC, this->_rotation);

    sysm::vector2 vAB = vB - vA;
    sysm::vector2 vBC = vC - vB;

    vA += this->_position;
    vB += this->_position;
    vC += this->_position;

    sysm::vector2 vAM = point - vA;
    sysm::vector2 vBM = point - vB;

    float dABAM = sysm::dot(vAB, vAM);
    float dBCBM = sysm::dot(vBC, vBM);

    return 0.0f <= dABAM && dABAM <= sysm::dot(vAB, vAB) && 0.0f <= dBCBM && dBCBM <= sysm::dot(vBC, vBC);
}
