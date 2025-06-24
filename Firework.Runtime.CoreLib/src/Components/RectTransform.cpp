#include "RectTransform.h"

#include <cmath>
#include <module/sys.Mathematics>

#include <GL/Renderer.h>
#include <Objects/Entity2D.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::Mathematics;
using namespace Firework::GL;

constexpr static void rotatePointAround(sysm::vector2& point, const sysm::vector2& rotateAround, float angle)
{
    float s = std::sinf(-angle * std::numbers::pi_v<float> / 180.0f);
    float c = std::cosf(-angle * std::numbers::pi_v<float> / 180.0f);

    float x = point.x - rotateAround.x;
    float y = point.y - rotateAround.y;

    point.x = x * c - y * s + rotateAround.x;
    point.y = x * s + y * c + rotateAround.y;
};
constexpr static void rotatePointAroundOrigin(sysm::vector2& point, float angle)
{
    return rotatePointAround(point, sysm::vector2::zero, angle);
};

namespace Firework::Internal
{
    RenderTransform renderTransformFromRectTransform(const RectTransform* const transform)
    {
        RenderTransform ret;
        ret.scale({ transform->rect().width() / 2.0f * transform->scale().x, transform->rect().height() / 2.0f * transform->scale().y, 0 });
        ret.translate({ (transform->rect().right + transform->rect().left) / 2.0f, (transform->rect().top + transform->rect().bottom) / 2.0f, 0.0f });
        ret.rotate(Renderer::fromEuler({ 0, 0, -transform->rotation() }));
        ret.translate({ transform->position().x, transform->position().y, 0.0f });
        return ret;
    }
} // namespace Firework::Internal

void RectTransform::setRect(const RectFloat& value)
{
    this->_dirty = true;

    RectFloat delta = value - this->_rect;
    this->_rect = value;

    auto recurse = [&](auto&& recurse, Entity2D* entity, RectFloat delta) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_dirty = true;
            RectFloat localDelta = delta * it->attachedRectTransform->_anchor;
            it->attachedRectTransform->_rect += localDelta;
            if (it->attachedRectTransform->_positionAnchor != RectFloat(0.0f))
            {
                it->attachedRectTransform->setLocalPosition(
                    it->attachedRectTransform->getLocalPosition() +
                    sysm::vector2(delta.right, delta.top) * sysm::vector2(it->attachedRectTransform->_positionAnchor.right, it->attachedRectTransform->_positionAnchor.top) +
                    sysm::vector2(delta.left, delta.bottom) * sysm::vector2(it->attachedRectTransform->_positionAnchor.left, it->attachedRectTransform->_positionAnchor.bottom));
            }
            recurse(recurse, it, localDelta);
        }
    };
    recurse(recurse, this->attachedEntity, delta);
}

void RectTransform::setPosition(sysm::vector2 value)
{
    this->_dirty = true;

    sysm::vector2 delta = value - this->_position;
    this->_position = value;

    auto setChildrenPositionRecursive = [&](auto&& setChildrenPositionRecursive, Entity2D* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_dirty = true;
            it->attachedRectTransform->_position += delta;
            setChildrenPositionRecursive(setChildrenPositionRecursive, it);
        }
    };
    setChildrenPositionRecursive(setChildrenPositionRecursive, this->attachedEntity);
}
void RectTransform::setRotation(float value)
{
    this->_dirty = true;

    float delta = value - this->_rotation;
    this->_rotation = value;

    auto setChildrenRotationRecursive = [&, this](auto&& setChildrenRotationRecursive, Entity2D* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_dirty = true;
            it->attachedRectTransform->_rotation += delta;
            rotatePointAround(it->attachedRectTransform->_position, this->_position, delta);
            setChildrenRotationRecursive(setChildrenRotationRecursive, it);
        }
    };
    setChildrenRotationRecursive(setChildrenRotationRecursive, this->attachedEntity);
}
void RectTransform::setScale(sysm::vector2 value)
{
    this->_dirty = true;

    sysm::vector2 delta = value / this->_scale;
    this->_scale = value;

    auto setChildrenScaleRecursive = [&, this](auto&& setChildrenScaleRecursive, Entity2D* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_dirty = true;
            it->attachedRectTransform->_scale *= delta;
            it->attachedRectTransform->_position = delta * (it->attachedRectTransform->_position - this->_position) + this->_position;
            setChildrenScaleRecursive(setChildrenScaleRecursive, it);
        }
    };
    setChildrenScaleRecursive(setChildrenScaleRecursive, this->attachedEntity);
}

sysm::vector2 RectTransform::getLocalPosition() const
{
    Entity2D* parent = this->attachedEntity->_parent;
    if (parent)
    {
        sysm::vector2 locPos = (this->_position - parent->attachedRectTransform->_position) / parent->attachedRectTransform->_scale;
        rotatePointAroundOrigin(locPos, -parent->attachedRectTransform->_rotation);
        return locPos;
    }
    else
        return this->_position;
}
void RectTransform::setLocalPosition(sysm::vector2 value)
{
    this->_dirty = true;

    Entity2D* parent = this->attachedEntity->_parent;
    sysm::vector2 delta;
    if (parent)
    {
        rotatePointAroundOrigin(value, parent->attachedRectTransform->_rotation);
        delta = parent->attachedRectTransform->_position + parent->attachedRectTransform->_scale * value - this->_position;
    }
    else
        delta = value - this->_position;
    this->_position += delta;

    auto setChildrenPositionRecursive = [&](auto&& setChildrenPositionRecursive, Entity2D* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_dirty = true;
            it->attachedRectTransform->_position += delta;
            setChildrenPositionRecursive(setChildrenPositionRecursive, it);
        }
    };
    setChildrenPositionRecursive(setChildrenPositionRecursive, this->attachedEntity);
}
float RectTransform::getLocalRotation() const
{
    Entity2D* parent = this->attachedEntity->_parent;
    return parent ? this->_rotation - parent->attachedRectTransform->_rotation : this->_rotation;
}
void RectTransform::setLocalRotation(float value)
{
    this->_dirty = true;

    Entity2D* parent = this->attachedEntity->_parent;
    float delta;
    if (parent)
        delta = parent->attachedRectTransform->_rotation + value - this->_rotation;
    else
        delta = value - this->_rotation;
    this->_rotation += delta;

    auto setChildrenRotationRecursive = [&, this](auto&& setChildrenRotationRecursive, Entity2D* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_dirty = true;
            it->attachedRectTransform->_rotation += delta;
            setChildrenRotationRecursive(setChildrenRotationRecursive, it);
            rotatePointAround(it->attachedRectTransform->_position, this->_position, delta);
        }
    };
    setChildrenRotationRecursive(setChildrenRotationRecursive, this->attachedEntity);
}
sysm::vector2 RectTransform::getLocalScale() const
{
    Entity2D* parent = this->attachedEntity->_parent;
    return parent ? this->_scale / parent->attachedRectTransform->_scale : this->_scale;
}
void RectTransform::setLocalScale(sysm::vector2 value)
{
    this->_dirty = true;

    Entity2D* parent = this->attachedEntity->_parent;
    sysm::vector2 delta;
    if (parent)
        delta = value * parent->attachedRectTransform->_scale / this->_scale;
    else
        delta = value / this->_scale;
    this->_scale *= delta;

    auto setChildrenScaleRecursive = [&, this](auto&& setChildrenScaleRecursive, Entity2D* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_dirty = true;
            it->attachedRectTransform->_scale *= delta;
            it->attachedRectTransform->_position = delta * (it->attachedRectTransform->_position - this->_position) + this->_position;
            setChildrenScaleRecursive(setChildrenScaleRecursive, it);
        }
    };
    setChildrenScaleRecursive(setChildrenScaleRecursive, this->attachedEntity);
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

    return 0.0f <= dABAM && dABAM <= Math::dot(vAB, vAB) && 0.0f <= dBCBM && dBCBM <= Math::dot(vBC, vBC);
}
