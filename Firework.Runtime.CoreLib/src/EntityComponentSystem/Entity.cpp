#include "Entity.h"

#include <EntityComponentSystem/EntityManagement.h>

using namespace Firework;
using namespace Firework::Internal;

std::shared_ptr<Entity> Entity::alloc(std::shared_ptr<Entity> parent)
{
    std::shared_ptr<Entity> ret = std::shared_ptr<Entity>(new Entity());
    ret->reparentAfterOrphan(parent);
    return ret;
}
Entity::~Entity()
{
    this->clear();
}
void Entity::clear()
{
    this->_childrenFront = nullptr;
    this->_childrenBack = nullptr;
    this->orphan();
    for (auto& [typeIndex, componentTable] : Entities::table) componentTable.erase(this);
}

void Entity::orphan() noexcept
{
    if (this->_parent)
    {
        if (this->_parent->_childrenFront.get() == this)
            this->_parent->_childrenFront = this->next;
        if (this->_parent->_childrenBack.get() == this)
            this->_parent->_childrenBack = this->prev;
    }
    else
    {
        if (Entities::front.get() == this)
            Entities::front = this->next;
        if (Entities::back.get() == this)
            Entities::back = this->prev;
    }

    std::shared_ptr<Entity> before = this->prev;
    std::shared_ptr<Entity> after = this->next;
    if (before)
        before->next = after;
    if (after)
        after->prev = before;
    this->prev = nullptr;
    this->next = nullptr;
    this->_parent = nullptr;
}
void Entity::reparentAfterOrphan(std::shared_ptr<Entity> newParent) noexcept
{
    std::shared_ptr<Entity> _this = shared_from_this();
    if (newParent)
    {
        if (newParent->_childrenBack == nullptr)
        {
            newParent->_childrenFront = _this;
            newParent->_childrenBack = _this;
        }
        else
        {
            newParent->_childrenBack->next = _this;
            this->prev = newParent->_childrenBack;
            newParent->_childrenBack = _this;
        }
        this->_parent = newParent;
    }
    else
    {
        if (Entities::back == nullptr)
        {
            Entities::front = _this;
            Entities::back = _this;
        }
        else
        {
            Entities::back->next = _this;
            this->prev = Entities::back;
            Entities::back = _this;
        }
    }
}
