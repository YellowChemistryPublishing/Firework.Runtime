#include "SceneManagement.h"

#include <Objects/Entity2D.h>
#include <Objects/Entity.h>

using namespace Firework;
using namespace Firework::Internal;

std::list<Internal::SceneMemoryChunk> SceneManager::existingScenes;

static struct Init {
    Init()
    {
        SceneManager::setSceneActive(SceneManager::createScene(), true);
    }
} init;

Scene::~Scene()
{
    {
        Entity2D* it = this->front2D;
        while (it)
        {
            auto itNext = it->next;
            delete it;
            it = itNext;
        }
    }
    {
        Entity* it = this->front;
        while (it)
        {
            auto itNext = it->next;
            delete it;
            it = itNext;
        }
    }
}

size_t Scene::index()
{
    size_t i = 0;
    for (auto it = SceneManager::existingScenes.begin(); it != SceneManager::existingScenes.end(); ++it)
    {
        if (reinterpret_cast<Scene*>(&it->data) == this)
            return i;
        ++i;
    }
    Debug::logError("Could not determine the index of the scene! Is the scene active?");
    return SIZE_MAX;
}
