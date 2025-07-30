#include "RectTransform.h"

#include <cmath>

#include <EntityComponentSystem/Entity.h>
#include <GL/Renderer.h>
#include <Library/Math.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;

inline static void rotatePointAround(glm::vec2& point, const glm::vec2& rotateAround, float angle)
{
    float s = std::sinf(-angle);
    float c = std::cosf(-angle);

    float x = point.x - rotateAround.x;
    float y = point.y - rotateAround.y;

    point.x = x * c - y * s + rotateAround.x;
    point.y = x * s + y * c + rotateAround.y;
};
inline static void rotatePointAroundOrigin(glm::vec2& point, float angle)
{
    return rotatePointAround(point, glm::vec2(0.0f), angle);
};

void RectTransform::setRect(const RectFloat& value)
{
    this->_dirty = true;
    this->matrixDirty = true;

    RectFloat delta = value - this->_rect;
    this->_rect = value;

    auto recurse = [&](auto&& recurse, Entity& entity, RectFloat delta) -> void
    {
        for (Entity& child : entity.children())
        {
            RectFloat localDelta = delta;
            if (std::shared_ptr<RectTransform> rectTransform = child.getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->matrixDirty = true;
                localDelta = delta * rectTransform->_anchor;
                rectTransform->_rect += localDelta;
                if (rectTransform->_positionAnchor != RectFloat(0.0f))
                {
                    rectTransform->setLocalPosition(rectTransform->getLocalPosition() +
                                                    glm::vec2(delta.right, delta.top) * glm::vec2(rectTransform->_positionAnchor.right, rectTransform->_positionAnchor.top) +
                                                    glm::vec2(delta.left, delta.bottom) * glm::vec2(rectTransform->_positionAnchor.left, rectTransform->_positionAnchor.bottom));
                }
            }
            recurse(recurse, child, localDelta);
        }
    };
    recurse(recurse, *this->attachedEntity, delta);
}

void RectTransform::setPosition(glm::vec2 value)
{
    this->_dirty = true;
    this->matrixDirty = true;

    glm::vec2 delta = value - this->_position;
    this->_position = value;

    auto setChildrenPositionRecursive = [&](auto&& setChildrenPositionRecursive, Entity& entity) -> void
    {
        for (Entity& child : entity.children())
        {
            if (std::shared_ptr<RectTransform> rectTransform = child.getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->matrixDirty = true;
                rectTransform->_position += delta;
            }
            setChildrenPositionRecursive(setChildrenPositionRecursive, child);
        }
    };
    setChildrenPositionRecursive(setChildrenPositionRecursive, *this->attachedEntity);
}
void RectTransform::setRotation(float value)
{
    this->_dirty = true;
    this->matrixDirty = true;

    float delta = value - this->_rotation;
    this->_rotation = value;

    auto setChildrenRotationRecursive = [&, this](auto&& setChildrenRotationRecursive, Entity& entity) -> void
    {
        for (Entity& child : entity.children())
        {
            if (std::shared_ptr<RectTransform> rectTransform = child.getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->matrixDirty = true;
                rectTransform->_rotation += delta;
                rotatePointAround(rectTransform->_position, this->_position, delta);
            }
            setChildrenRotationRecursive(setChildrenRotationRecursive, child);
        }
    };
    setChildrenRotationRecursive(setChildrenRotationRecursive, *this->attachedEntity);
}
void RectTransform::setScale(glm::vec2 value)
{
    this->_dirty = true;
    this->matrixDirty = true;

    glm::vec2 delta = value / this->_scale;
    this->_scale = value;

    auto setChildrenScaleRecursive = [&, this](auto&& setChildrenScaleRecursive, Entity& entity) -> void
    {
        for (Entity& child : entity.children())
        {
            if (std::shared_ptr<RectTransform> rectTransform = child.getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->matrixDirty = true;
                rectTransform->_scale *= delta;
                rectTransform->_position = delta * (rectTransform->_position - this->_position) + this->_position;
            }
            setChildrenScaleRecursive(setChildrenScaleRecursive, child);
        }
    };
    setChildrenScaleRecursive(setChildrenScaleRecursive, *this->attachedEntity);
}

std::shared_ptr<RectTransform> RectTransform::parent() const
{
    for (std::shared_ptr<Entity> parent = this->attachedEntity->parent; parent; parent = parent->parent)
    {
        if (std::shared_ptr<RectTransform> rectTransform = parent->getComponent<RectTransform>())
            return rectTransform;
    }
    return nullptr;
}

glm::vec2 RectTransform::getLocalPosition() const
{
    std::shared_ptr<RectTransform> parent = this->parent();
    if (parent)
    {
        glm::vec2 locPos = (this->_position - parent->_position) / parent->_scale;
        rotatePointAroundOrigin(locPos, -parent->_rotation);
        return locPos;
    }
    else
        return this->_position;
}
void RectTransform::setLocalPosition(glm::vec2 value)
{
    this->_dirty = true;
    this->matrixDirty = true;

    std::shared_ptr<RectTransform> parent = this->parent();
    glm::vec2 delta;
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
        for (Entity& child : entity.children())
        {
            if (std::shared_ptr<RectTransform> rectTransform = child.getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->matrixDirty = true;
                rectTransform->_position += delta;
            }
            setChildrenPositionRecursive(setChildrenPositionRecursive, child);
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
    this->matrixDirty = true;

    std::shared_ptr<RectTransform> parent = this->parent();
    float delta;
    if (parent)
        delta = parent->_rotation + value - this->_rotation;
    else
        delta = value - this->_rotation;
    this->_rotation += delta;

    auto setChildrenRotationRecursive = [&, this](auto&& setChildrenRotationRecursive, Entity& entity) -> void
    {
        for (Entity& child : entity.children())
        {
            if (std::shared_ptr<RectTransform> rectTransform = child.getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->matrixDirty = true;
                rectTransform->_rotation += delta;
                rotatePointAround(rectTransform->_position, this->_position, delta);
            }
            setChildrenRotationRecursive(setChildrenRotationRecursive, child);
        }
    };
    setChildrenRotationRecursive(setChildrenRotationRecursive, *this->attachedEntity);
}
glm::vec2 RectTransform::getLocalScale() const
{
    std::shared_ptr<RectTransform> parent = this->parent();
    return parent ? this->_scale / parent->_scale : this->_scale;
}
void RectTransform::setLocalScale(glm::vec2 value)
{
    this->_dirty = true;
    this->matrixDirty = true;

    std::shared_ptr<RectTransform> parent = this->parent();
    glm::vec2 delta;
    if (parent)
        delta = value * parent->_scale / this->_scale;
    else
        delta = value / this->_scale;
    this->_scale *= delta;

    auto setChildrenScaleRecursive = [&, this](auto&& setChildrenScaleRecursive, Entity& entity) -> void
    {
        for (Entity& child : entity.children())
        {
            if (std::shared_ptr<RectTransform> rectTransform = child.getComponent<RectTransform>())
            {
                rectTransform->_dirty = true;
                rectTransform->matrixDirty = true;
                rectTransform->_scale *= delta;
                rectTransform->_position = delta * (rectTransform->_position - this->_position) + this->_position;
            }
            setChildrenScaleRecursive(setChildrenScaleRecursive, child);
        }
    };
    setChildrenScaleRecursive(setChildrenScaleRecursive, *this->attachedEntity);
}

const glm::mat4& RectTransform::matrix()
{
    if (this->matrixDirty)
    {
        glm::mat4 tf = glm::translate(glm::mat4(1.0f), glm::vec3(this->_position.x, this->_position.y, 0.0f));
        tf = glm::rotate(tf, -this->_rotation, LinAlgConstants::forward);
        tf = glm::translate(tf, glm::vec3((this->_rect.right + this->_rect.left) / 2.0f, (this->_rect.top + this->_rect.bottom) / 2.0f, 0.0f));
        tf = glm::scale(tf, glm::vec3(this->_rect.width() * this->_scale.x, this->_rect.height() * this->_scale.y, 0.0f));
        this->_matrix = tf;

        this->matrixDirty = false;
    }

    return this->_matrix;
}

bool RectTransform::queryPointIn(const glm::vec2& point)
{
    float rL = this->_rect.left * this->_scale.x;
    float rB = this->_rect.bottom * this->_scale.y;

    glm::vec2 vA(this->_rect.right * this->_scale.x, rB);
    rotatePointAroundOrigin(vA, this->_rotation);
    glm::vec2 vB(rL, rB);
    rotatePointAroundOrigin(vB, this->_rotation);
    glm::vec2 vC(rL, this->_rect.top * this->_scale.y);
    rotatePointAroundOrigin(vC, this->_rotation);

    glm::vec2 vAB = vB - vA;
    glm::vec2 vBC = vC - vB;

    vA += this->_position;
    vB += this->_position;
    vC += this->_position;

    glm::vec2 vAM = point - vA;
    glm::vec2 vBM = point - vB;

    float dABAM = glm::dot(vAB, vAM);
    float dBCBM = glm::dot(vBC, vBM);

    return 0.0f <= dABAM && dABAM <= glm::dot(vAB, vAB) && 0.0f <= dBCBM && dBCBM <= glm::dot(vBC, vBC);
}
