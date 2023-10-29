#include "Entity2D.h"

#include <EntityComponentSystem/EntityManagement.h>
#include <EntityComponentSystem/SceneManagement.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::Mathematics;

#define thisRect this->attachedRectTransform

Entity2D::Entity2D() : attachedRectTransform(new RectTransform())
{
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    if (mainScene->front2D)
        this->insertAfter(mainScene->back2D);
    else
    {
        mainScene->front2D = this;
        mainScene->back2D = this;
    }
    this->attachedScene = mainScene;
}
Entity2D::Entity2D(Entity2D* parent) : attachedRectTransform(new RectTransform())
{
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    this->parent = parent;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    if (mainScene->front2D)
        this->insertAfter(mainScene->back2D);
    else
    {
        mainScene->front2D = this;
        mainScene->back2D = this;
    }
    this->attachedScene = mainScene;
}
Entity2D::Entity2D(const Vector2& localPosition, float localRotation, Entity2D* parent) : attachedRectTransform(new RectTransform())
{
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    this->parent = parent;
    thisRect->localPosition = localPosition;
    thisRect->localRotation = localRotation;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    if (mainScene->front2D)
        this->insertAfter(mainScene->back2D);
    else
    {
        mainScene->front2D = this;
        mainScene->back2D = this;
    }
    this->attachedScene = mainScene;
}
Entity2D::Entity2D(const Vector2& localPosition, float localRotation, const Vector2& scale, Entity2D* parent) : attachedRectTransform(new RectTransform())
{
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    this->parent = parent;
    thisRect->localPosition = localPosition;
    thisRect->localRotation = localRotation;
    thisRect->scale = scale;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    if (mainScene->front2D)
        this->insertAfter(mainScene->back2D);
    else
    {
        mainScene->front2D = this;
        mainScene->back2D = this;
    }
    this->attachedScene = mainScene;
}
Entity2D::~Entity2D()
{
    auto it = this->childrenFront;
    while (it != nullptr)
    {
        auto itNext = it->next;
        delete it;
        it = itNext;
    }

    for (auto it = EntityManager2D::existingComponents.begin(); it != EntityManager2D::existingComponents.end(); ++it)
    {
        auto component = EntityManager2D::components.find(std::make_pair(this, it->first));
        if (component != EntityManager2D::components.end())
        {
            delete component->second;
            EntityManager2D::components.erase(component);
            if (--it->second == 0)
                EntityManager2D::existingComponents.erase(it);
        }
    }
    delete this->attachedRectTransform;

    this->eraseFromImplicitList();
}

void Entity2D::insertAfter(Entity2D* entity)
{
    if (entity->next)
    {
        entity->next->prev = this;
        this->next = entity->next;
    }
    else
    {
        this->next = nullptr;
        entity->attachedScene->back2D = this;
    }
    entity->next = this;
    this->prev = entity;
}
void Entity2D::moveAfter(Entity2D* entity)
{
    this->eraseFromImplicitList();
    this->insertAfter(entity);
}
void Entity2D::eraseFromImplicitList()
{
    if (this->prev)
        this->prev->next = this->next;
    else
    {  
        if (this->_parent)
            this->_parent->childrenFront = this->next;
        else this->attachedScene->front2D = this->next;
    }
    if (this->next)
        this->next->prev = this->prev;
    else
    {
        if (this->_parent)
            this->_parent->childrenBack = this->prev;
        else this->attachedScene->back2D = this->prev;
    }
}

void Entity2D::setParent(Entity2D* value)
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
        if (this->attachedScene->back2D)
            this->insertAfter(this->attachedScene->back2D);
        else
        {
            this->attachedScene->front2D = this;
            this->attachedScene->back2D = this;
            this->prev = nullptr;
            this->next = nullptr;
        }
    }
}

/*void Entity2D::setIndex(size_t value)
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
