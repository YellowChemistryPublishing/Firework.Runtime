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
        const Property<const std::wstring&, std::wstring> name
        {{
            [this]() -> const std::wstring& { return this->_name; },
            [this](std::wstring value) { this->_name = std::move(value); }
        }};
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

        inline static Scene* createScene()
        {
            SceneManager::existingScenes.emplace_back();
            return new(SceneManager::existingScenes.back().data) Scene;
        }
        inline static void destroyScene(Scene* scene)
        {
            auto it = scene->it;
            scene->~Scene();
            SceneManager::existingScenes.erase(it);
        }

        inline static void setSceneActive(Scene* scene, bool active)
        {
            scene->active = active;
        }
        inline static void setMainScene(Scene* scene)
        {
            scene->active = true;
            SceneManager::existingScenes.splice(SceneManager::existingScenes.begin(), SceneManager::existingScenes, scene->it);
        }

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
    {
    }
}