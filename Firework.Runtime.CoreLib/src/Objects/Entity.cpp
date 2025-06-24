#include "Entity.h"

#include <Components/Transform.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <EntityComponentSystem/SceneManagement.h>

using namespace Firework;
using namespace Firework::Internal;

#define thisTransform this->attachedTransform

Entity::Entity() : attachedTransform(new Transform)
{
    this->attachedTransform->attachedEntity = this;
    this->attachedTransform->attachedTransform = this->attachedTransform;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    if (mainScene->front)
        this->insertAfter(mainScene->back);
    else
    {
        mainScene->front = this;
        mainScene->back = this;
    }
    this->attachedScene = mainScene;
}
Entity::Entity(Entity* parent) : attachedTransform(new Transform)
{
    this->attachedTransform->attachedEntity = this;
    this->attachedTransform->attachedTransform = this->attachedTransform;
    this->parent = parent;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    if (mainScene->front)
        this->insertAfter(mainScene->back);
    else
    {
        mainScene->front = this;
        mainScene->back = this;
    }
    this->attachedScene = mainScene;
}
Entity::Entity(const sysm::vector3& localPosition, const sysm::quaternion& localRotation, Entity* parent) : attachedTransform(new Transform)
{
    this->attachedTransform->attachedEntity = this;
    this->attachedTransform->attachedTransform = this->attachedTransform;
    this->parent = parent;
    thisTransform->localPosition = localPosition;
    thisTransform->localRotation = localRotation;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    if (mainScene->front)
        this->insertAfter(mainScene->back);
    else
    {
        mainScene->front = this;
        mainScene->back = this;
    }
    this->attachedScene = mainScene;
}
Entity::Entity(const sysm::vector3& localPosition, const sysm::quaternion& localRotation, const sysm::vector3& scale, Entity* parent) : attachedTransform(new Transform)
{
    this->attachedTransform->attachedEntity = this;
    this->attachedTransform->attachedTransform = this->attachedTransform;
    this->parent = parent;
    thisTransform->localPosition = localPosition;
    thisTransform->localRotation = localRotation;
    thisTransform->scale = scale;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    if (mainScene->front)
        this->insertAfter(mainScene->back);
    else
    {
        mainScene->front = this;
        mainScene->back = this;
    }
    this->attachedScene = mainScene;
}
Entity::~Entity()
{
    auto it = this->childrenFront;
    while (it != nullptr)
    {
        auto itNext = it->next;
        delete it;
        it = itNext;
    }

    for (auto it = EntityManager::existingComponents.begin(); it != EntityManager::existingComponents.end(); ++it)
    {
        auto component = EntityManager::components.find(std::make_pair(this, it->first));
        if (component != EntityManager::components.end())
        {
            delete component->second;
            EntityManager::components.erase(component);
            if (--it->second == 0)
                EntityManager::existingComponents.erase(it);
        }
    }
    delete this->attachedTransform;

    this->eraseFromImplicitList();
}

void Entity::insertAfter(Entity* entity)
{
    if (entity->next)
    {
        entity->next->prev = this;
        this->next = entity->next;
    }
    else
    {
        this->next = nullptr;
        entity->attachedScene->back = this;
    }
    entity->next = this;
    this->prev = entity;
}
void Entity::moveAfter(Entity* entity)
{
    this->eraseFromImplicitList();
    this->insertAfter(entity);
}
void Entity::eraseFromImplicitList()
{
    if (this->prev)
        this->prev->next = this->next;
    else
    {  
        if (this->_parent)
            this->_parent->childrenFront = this->next;
        else this->attachedScene->front = this->next;
    }
    if (this->next)
        this->next->prev = this->prev;
    else
    {
        if (this->_parent)
            this->_parent->childrenBack = this->prev;
        else this->attachedScene->back = this->prev;
    }
}

void Entity::setParent(Entity* value)
{
    this->eraseFromImplicitList();
    
    this->_parent = value;
    if (this->_parent)
    {
        if (this->_parent->childrenBack)
            this->insertAfter(this->_parent->childrenBack);
        else
        {
            this->_parent->childrenFront = this;
            this->_parent->childrenBack = this;
            this->prev = nullptr;
            this->next = nullptr;
        }
    }
    else
    {
        if (this->attachedScene->back)
            this->insertAfter(this->attachedScene->back);
        else
        {
            this->attachedScene->front = this;
            this->attachedScene->back = this;
            this->prev = nullptr;
            this->next = nullptr;
        }
    }
}

/*void Entity::setIndex(size_t value)
{
    if (value < this->attachedScene->entities.size())
    {
        this->attachedScene->entities.splice
        (
            std::next(this->attachedScene->entities.begin(), value),
            this->attachedScene->entities, this->it
        );
        this->_index = value;
        auto it = std::next(this->it);
        while (it != this->attachedScene->entities.end())
        {
            ++(*it)->_index;
            ++it;
        }
    }
}*/
