#include "Entity.h"

#include <EntityComponentSystem/EntityManagement.h>

using namespace Firework;
using namespace Firework::Internal;

Entity::Entity(std::shared_ptr<Entity> parent)
{
    std::shared_ptr<Entity> _this = std::shared_ptr<Entity>(this);
    this->reparentAfterOrphan(parent);
}
Entity::~Entity()
{
    for (auto& [typeIndex, componentTable] : Entities::table) componentTable.erase(this);
    this->orphan();
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
void Entity::reparentAfterOrphan(std::shared_ptr<Entity> parent) noexcept
{
    std::shared_ptr<Entity> _this = shared_from_this();
    if (parent)
    {
        if (parent->_childrenBack == nullptr)
        {
            parent->_childrenFront = _this;
            parent->_childrenBack = _this;
        }
        else
        {
            parent->_childrenBack->next = _this;
            this->prev = parent->_childrenBack;
            parent->_childrenBack = _this;
        }
        this->_parent = parent;
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
