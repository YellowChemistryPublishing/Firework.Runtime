#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <concepts>
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

///
/// The documentation in this file is incomplete, because the API is work-in-progress.
/// FIXME: Please fix.
///

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

        /// @brief Retrieves the 3D transform component associated with this entity.
        /// @return Transform component.
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

        /// @brief Add a component to this entity.
        /// @tparam T Any 3D component type, with the exception of Transform. (Transform is a default component, 3D entities are _always_ guaranteed to have one.)
        /// @return Component added to entity.
        /// @retval - ```nullptr```: The component was not able to be added to the entity.
        /// @retval - Otherwise, pointer to the added component.
        /// @note Main thread only.
        template <typename T>
        requires !std::same_as<T, Transform> && std::derived_from<T, Internal::Component>
        inline T* addComponent();
        /// @brief Retrieve a component from this entity.
        /// @tparam T Any 3D component type.
        /// @return Component attached to entity.
        /// @retval - ```nullptr```: The component does not exist on this entity.
        /// @retval - Otherwise, pointer to attached component.
        /// @note Main thread only.
        template <typename T>
        requires std::derived_from<T, Internal::Component>
        [[nodiscard]] inline T* getComponent();
        /// @brief Remove a component from this entity. Does nothing if queried component doesn't exist on this entity.
        /// @tparam T Any 3D component type, with the exception of Transform. (Transform is a default component, 3D entities must _always_ have one.)
        /// @note Main thread only.
        template <typename T>
        requires !std::same_as<T, Transform> && std::derived_from<T, Internal::Component>
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