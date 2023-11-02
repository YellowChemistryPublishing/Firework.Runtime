#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <list>
#include <string>

#include <Library/Property.h>
 
namespace Firework
{
    class SceneManager;
    class EntityManager2D;
    class EntityManager;
    class Entity2D;
    class Entity;
    class Debug;

    namespace Internal
    {
        class CoreEngine;
        struct SceneMemoryChunk;
    }

    class __firework_corelib_api Scene final
    {
        inline Scene();
        ~Scene();

        Entity2D* front2D = nullptr;
        Entity2D* back2D = nullptr;
        Entity* front = nullptr;
        Entity* back = nullptr;
        
        std::list<Internal::SceneMemoryChunk>::iterator it;
        bool active = false;
        std::wstring _name = L"Untitled";
    public:
        /// @brief [Property] The name of the scene.
        /// @param value ```std::wstring```
        /// @return ```const std::wstring&```
        /// @note Main thread only.
        const Property<const std::wstring&, std::wstring> name
        {{
            [this]() -> const std::wstring& { return this->_name; },
            [this](std::wstring value) { this->_name = std::move(value); }
        }};
        /// @brief Retrieves the index of the scene. Index ```0``` is first.
        /// @warning This function runs in O(n) time, for n number of scenes. Be reasonable with it!
        /// @return Index of scene.
        /// @retval - ```SIZE_MAX```: An error in determining the scene index occurred.
        /// @retval - Othewise, the index of the scene.
        /// @note Main thread only.
        size_t index();

        friend class Firework::Internal::CoreEngine;
        friend class Firework::SceneManager;
        friend class Firework::EntityManager2D;
        friend class Firework::EntityManager;
        friend class Firework::Entity2D;
        friend class Firework::Entity;
        friend class Firework::Debug;
    };

    namespace Internal
    {
        struct SceneMemoryChunk
        {
            alignas(Scene) char data[sizeof(Scene)];
        };
    }
    
    class __firework_corelib_api SceneManager final
    {
        static std::list<Internal::SceneMemoryChunk> existingScenes;
    public:
        SceneManager() = delete;

        /// @brief Creates a scene
        /// @return New default-constructed scene.
        /// @note Main thread only.
        inline static Scene* createScene()
        {
            SceneManager::existingScenes.emplace_back();
            return new(SceneManager::existingScenes.back().data) Scene;
        }
        /// @brief Destroys an existing scene, including all its entities and components.
        /// @param scene Scene to destroy.
        /// @note Main thread only.
        inline static void destroyScene(Scene* scene)
        {
            auto it = scene->it;
            scene->~Scene();
            SceneManager::existingScenes.erase(it);
        }

        /// @brief Sets a scene to an active state.
        /// @param scene Scene to set to an active state.
        /// @param active Whether the scene should be set to active or not.
        /// @note Main thread only.
        inline static void setSceneActive(Scene* scene, bool active)
        {
            scene->active = active;
        }
        /// @brief
        /// Sets a scene as the main scene. By default, unless specified, entities will be added to this scene.
        /// This also implicitly sets the scene to the index of ```0```, invalidating all existing stored indices.
        /// @param scene Scene to set as the main scene.
        /// @note Main thread only.
        inline static void setMainScene(Scene* scene)
        {
            scene->active = true;
            SceneManager::existingScenes.splice(SceneManager::existingScenes.begin(), SceneManager::existingScenes, scene->it);
        }

        /// @brief Retrieves a scene by its index.
        /// @warning This function runs in O(n) time, for n number of scenes. Invoke wisely!
        /// @param index Index of the queried scene.
        /// @return Scene with queried index.
        /// @retval - ```nullptr```: The index was out of bounds.
        /// @retval - Otherwise, the scene with the queried index.
        /// @note Main thread only.
        inline static Scene* getSceneByIndex(size_t index)
        {
            if (index < SceneManager::existingScenes.size())
                return reinterpret_cast<Scene*>(&std::next(SceneManager::existingScenes.begin(), index)->data);
            else return nullptr;
        }

        friend class Firework::Internal::CoreEngine;
        friend class Firework::Scene;
        friend class Firework::EntityManager2D;
        friend class Firework::EntityManager;
        friend class Firework::Entity2D;
        friend class Firework::Entity;
        friend class Firework::Debug;
    };

    Scene::Scene() : it(--SceneManager::existingScenes.end())
    { }
}