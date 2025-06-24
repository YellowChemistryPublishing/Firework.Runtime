#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <module/sys.Mathematics>
#include <robin_hood.h>
#include <type_traits>
#include <vector>

#include <Components/RectTransform.h>
#include <Core/Debug.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <Library/Hash.h>
#include <Library/TypeInfo.h>
#include <Objects/Component2D.h>

///
/// The documentation in this file is incomplete, because the API is work-in-progress.
/// FIXME: Please fix.
///

namespace Firework
{
    class Scene;
    class SceneManager;
    class Entity2D;
    class EntityManager;
    class Debug;

    namespace Internal
    {
        class CoreEngine;
    }

    class __firework_corelib_api Entity2D final : public Internal::Object
    {
        Scene* attachedScene;
        RectTransform* attachedRectTransform;

        Entity2D* next = nullptr;
        Entity2D* prev = nullptr;

        Entity2D* _parent = nullptr;
        Entity2D* childrenFront = nullptr;
        Entity2D* childrenBack = nullptr;

        // size_t getIndex();
        // void setIndex(size_t value);

        void insertBefore(Entity2D* entity);
        void insertAfter(Entity2D* entity);
        void eraseFromImplicitList();

        void setParent(Entity2D* value);
    public:
        std::wstring name = L"Unnamed";

        Entity2D();
        Entity2D(Entity2D* parent);
        Entity2D(const sysm::vector2& localPosition, float localRotation, Entity2D* parent = nullptr);
        Entity2D(const sysm::vector2& localPosition, float localRotation, const sysm::vector2& scale, Entity2D* parent = nullptr);
        ~Entity2D() override;

        /// @brief Retrieves the 2D transform component associated with this entity.
        /// @return RectTransform component.
        inline RectTransform* rectTransform()
        {
            return this->attachedRectTransform;
        }

        const Property<Entity2D*, Entity2D*> parent { [this]() -> Entity2D* { return this->_parent; }, [this](Entity2D* value)
        {
            this->setParent(value);
        } };
        inline std::vector<Entity2D*> children() const
        {
            std::vector<Entity2D*> ret;
            for (auto it = this->childrenFront; it != nullptr; ++it) ret.push_back(it);
            return ret;
        }

        void moveBefore(Entity2D* entity);
        void moveAfter(Entity2D* entity);

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
        friend class Firework::EntityManager2D;
        friend class Firework::Internal::Component2D;
        friend class Firework::RectTransform;
        friend class Firework::Internal::CoreEngine;
        friend class Firework::Debug;
    };
} // namespace Firework