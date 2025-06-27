#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <list>
#include <string>

#include <Library/Property.h>

namespace Firework
{
    class SceneManager;
    class EntityManager;
    class Entity;
    class Debug;

    namespace Internal
    {
        class CoreEngine;
    } // namespace Internal

    class __firework_corelib_api Scene final
    {
        Scene() = default;

        Entity* front = nullptr;
        Entity* back = nullptr;

        std::list<Scene>::iterator it;
        bool active = false;
    public:
        friend class std::allocator<Scene>;

        friend class Firework::Internal::CoreEngine;
        friend class Firework::SceneManager;
        friend class Firework::EntityManager;
        friend class Firework::Entity;
        friend class Firework::Debug;

    };

    /// @brief Static class containing functionality relevant to scene management.
    class __firework_corelib_api SceneManager final
    {
        static std::list<Scene> existingScenes;
    public:
        SceneManager() = delete;

        /// @brief Creates a scene
        /// @return New default-constructed scene.
        /// @note Main thread only.
        inline static Scene* createScene()
        {
            SceneManager::existingScenes.push_back(Scene());
            Scene& ret = SceneManager::existingScenes.back();
            ret.it = --SceneManager::existingScenes.end();
            return &ret;
        }
        /// @brief Destroys an existing scene, including all its entities and components.
        /// @param scene Scene to destroy.
        /// @note Main thread only.
        inline static void destroyScene(Scene* scene)
        {
            SceneManager::existingScenes.erase(scene->it);
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

        friend class Firework::Internal::CoreEngine;
        friend class Firework::Scene;
        friend class Firework::EntityManager;
        friend class Firework::Entity;
        friend class Firework::Debug;
    };
} // namespace Firework