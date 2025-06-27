#include "Entity.h"

#include <EntityComponentSystem/EntityManagement.h>

using namespace Firework;
using namespace Firework::Internal;

Entity::Entity(Entity* parent)
{
    this->reparentAfterOrphan(parent);
}
Entity::~Entity()
{
    for (auto& [typeIndex, componentTable] : Entities::table) componentTable.erase(this);
}

void Entity::orphan()
{
    Entity* before = this->prev;
    Entity* after = this->next;
    if (before)
        before->next = after;
    if (after)
        after->prev = before;
    this->prev = nullptr;
    this->next = nullptr;
}
void Entity::reparentAfterOrphan(Entity* parent)
{
    if (!parent)
    {
        if (Entities::back == nullptr)
        {
            Entities::front = this;
            Entities::back = this;
        }
        else
        {
            Entities::back->next = this;
            this->prev = Entities::back;
            Entities::back = this;
        }
    }
    else
    {
        if (parent->_childrenBack == nullptr)
        {
            parent->_childrenFront = this;
            parent->_childrenBack = this;
        }
        else
        {
            parent->_childrenBack->next = this;
            this->prev = parent->_childrenBack;
            parent->_childrenBack = this;
        }
        this->_parent = parent;
    }
}
