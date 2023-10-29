#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <vector>
#include <type_traits>
#include <robin_hood.h>

#include <Mathematics.h>
#include <Core/Debug.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <Objects/Component.h>
#include <Library/ManagedArray.h>
#include <Library/Hash.h>
#include <Library/TypeInfo.h>

namespace Firework
{
    class Scene;
    class SceneManager;
    class Entity;
    class EntityManager;
    class Debug;
    
    namespace Internal
    {
        class CoreEngine;
    }

    class __firework_corelib_api Entity final : public Internal::Object
    {
        Scene* attachedScene;
        Transform* attachedTransform;

        Entity* next = nullptr;
        Entity* prev = nullptr;

        Entity* _parent = nullptr;
        Entity* childrenFront = nullptr;
        Entity* childrenBack = nullptr;

        //size_t getIndex();
        //void setIndex(size_t value);

        void insertBefore(Entity* entity);
        void insertAfter(Entity* entity);
        void eraseFromImplicitList();

        void setParent(Entity* value);
    public:
        std::wstring name = L"Unnamed";

        Entity();
        Entity(Entity* parent);
        Entity(const Mathematics::Vector3& localPosition, const Mathematics::Quaternion& localRotation, Entity* parent = nullptr);
        Entity(const Mathematics::Vector3& localPosition, const Mathematics::Quaternion& localRotation, const Mathematics::Vector3& scale, Entity* parent = nullptr);
        ~Entity() override;

        inline Transform* transform()
        {
            return this->attachedTransform;
        }
        
        const Property<Entity*, Entity*> parent
        {{
            [this]() -> Entity* { return this->_parent; },
            [this](Entity* value) { this->setParent(value); }
        }};
        inline std::vector<Entity*> children() const
        {
            std::vector<Entity*> ret;
            for (auto it = this->childrenFront; it != nullptr; ++it)
                ret.push_back(it);
            return ret;
        }

        void moveBefore(Entity* entity);
        void moveAfter(Entity* entity);

        /*Property<size_t, size_t> index
        {{
            [this]() -> size_t { return this->_index; },
            [this](size_t value) { this->setIndex(value); }
        }};*/

        template <typename T>
        inline T* addComponent();
        template <typename T>
        inline T* getComponent();
        template <typename T>
        inline void removeComponent();

        friend class Firework::SceneManager;
        friend class Firework::Scene;
        friend class Firework::EntityManager;
        friend class Firework::Internal::Component;
        friend class Firework::Transform;
        friend class Firework::Internal::CoreEngine;
        friend class Firework::Debug;
    };
}