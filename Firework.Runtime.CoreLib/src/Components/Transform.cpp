#include "Transform.h"

#include <cassert>

#include <Objects/Entity.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::Mathematics;
using namespace Firework::GL;

constexpr static auto rotatePointAround = [](Vector3& point, const Vector3& rotateAround, const Quaternion& rot) -> void
{
    Vector3 delta = point - rotateAround;
    delta = (rot * Quaternion(0.0f, delta) * rot.conjugate()).vector();
    point = rotateAround + delta;
};
constexpr static auto rotatePointAroundOrigin = [](Vector3& point, const Quaternion& rot) -> void
{
    point = (rot * Quaternion(0.0f, point) * rot.conjugate()).vector();
};

namespace Firework::Internal
{
    RenderTransform renderTransformFromTransform(Transform* const transform)
    {
        RenderTransform ret;
        ret.scale(transform->scale());
        ret.rotate(transform->rotation().conjugate());
        assert(transform->rotation().norm2() == 1.0f && "Rotation quaterions are required to be normalized!");
        ret.translate(transform->position());
        return ret;
    }
}

void Transform::setPosition(Vector3 value)
{
    Vector3 delta = value - this->_position;
    this->_position = value;

    auto setChildrenPositionRecursive = [&](auto&& setChildrenPositionRecursive, Entity* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedTransform->_position += delta;
            setChildrenPositionRecursive(setChildrenPositionRecursive, it);
        }
    };
    setChildrenPositionRecursive(setChildrenPositionRecursive, this->attachedEntity);
}
void Transform::setRotation(Quaternion value)
{
    Quaternion delta = value * this->_rotation.conjugate().unit();
    this->_rotation = value.unit();

    auto setChildrenRotationRecursive = [&, this](auto&& setChildrenRotationRecursive, Entity* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedTransform->_rotation *= delta;
            rotatePointAround(it->attachedTransform->_position, this->_position, delta);
            setChildrenRotationRecursive(setChildrenRotationRecursive, it);
        }
    };
    setChildrenRotationRecursive(setChildrenRotationRecursive, this->attachedEntity);
}
void Transform::setScale(Vector3 value)
{
    Vector3 delta = value / this->_scale;
    this->_scale = value;

    auto setChildrenScaleRecursive = [&, this](auto&& setChildrenScaleRecursive, Entity* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedTransform->_scale *= delta;
            it->attachedTransform->_position = delta * (it->attachedTransform->_position - this->_position) + this->_position;
            setChildrenScaleRecursive(setChildrenScaleRecursive, it);
        }
    };
    setChildrenScaleRecursive(setChildrenScaleRecursive, this->attachedEntity);
}

Vector3 Transform::getLocalPosition() const
{
    Entity* parent = this->attachedEntity->_parent;
    if (parent)
    {
        Vector3 locPos = (this->_position - parent->attachedTransform->_position) / parent->attachedTransform->_scale;
        assert(parent->attachedTransform->_rotation.norm2() == 1.0f && "Rotation quaterions are required to be normalized!");
        rotatePointAroundOrigin(locPos, parent->attachedTransform->_rotation.conjugate());
        return locPos;
    }
    else return this->_position;
}
void Transform::setLocalPosition(Vector3 value)
{
    Entity* parent = this->attachedEntity->_parent;
    Vector3 delta;
    if (parent)
    {
        rotatePointAroundOrigin(value, parent->attachedTransform->_rotation);
        delta = parent->attachedTransform->_position + parent->attachedTransform->_scale * value - this->_position;
    }
    else delta = value - this->_position;
    this->_position += delta;

    auto setChildrenPositionRecursive = [&](auto&& setChildrenPositionRecursive, Entity* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedTransform->_position += delta;
            setChildrenPositionRecursive(setChildrenPositionRecursive, it);
        }
    };
    setChildrenPositionRecursive(setChildrenPositionRecursive, this->attachedEntity);
}
Quaternion Transform::getLocalRotation() const
{
    Entity* parent = this->attachedEntity->_parent;
    assert(parent->attachedTransform->_rotation.norm2() == 1.0f && "Rotation quaterions are required to be normalized!");
    return parent ? this->_rotation * parent->attachedTransform->_rotation.conjugate() : this->_rotation;
}
void Transform::setLocalRotation(Quaternion value)
{
    Entity* parent = this->attachedEntity->_parent;
    Quaternion delta;
    assert(value.norm2() == 1.0f && this->_rotation.norm2() == 1.0f && "Rotation quaterions are required to be normalized!");
    if (parent)
    {
        assert(parent->attachedTransform->_rotation.norm2() == 1.0f && "Rotation quaterions are required to be normalized!");
        delta = (parent->attachedTransform->_rotation * value * this->_rotation.conjugate()).unit();
    }
    else delta = (value * this->_rotation.conjugate()).unit();
    this->_rotation *= delta;

    auto setChildrenRotationRecursive = [&, this](auto&& setChildrenRotationRecursive, Entity* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            assert(it->attachedTransform->_rotation.norm2() == 1.0f && "Rotation quaterions are required to be normalized!");
            it->attachedTransform->_rotation *= delta;
            setChildrenRotationRecursive(setChildrenRotationRecursive, it);
            rotatePointAround(it->attachedTransform->_position, this->_position, delta);
        }
    };
    setChildrenRotationRecursive(setChildrenRotationRecursive, this->attachedEntity);
}
Vector3 Transform::getLocalScale() const
{
    Entity* parent = this->attachedEntity->_parent;
    return parent ? this->_scale / parent->attachedTransform->_scale : this->_scale;
}
void Transform::setLocalScale(Vector3 value)
{
    Entity* parent = this->attachedEntity->_parent;
    Vector3 delta;
    if (parent)
        delta = value * parent->attachedTransform->_scale / this->_scale;
    else delta = value / this->_scale;
    this->_scale *= delta;

    auto setChildrenScaleRecursive = [&, this](auto&& setChildrenScaleRecursive, Entity* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedTransform->_scale *= delta;
            it->attachedTransform->_position = delta * (it->attachedTransform->_position - this->_position) + this->_position;
            setChildrenScaleRecursive(setChildrenScaleRecursive, it);
        }
    };
    setChildrenScaleRecursive(setChildrenScaleRecursive, this->attachedEntity);
}