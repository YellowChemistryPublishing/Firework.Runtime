#include "SceneManagement.h"

#include <EntityComponentSystem/Entity.h>

using namespace Firework;
using namespace Firework::Internal;

std::list<Scene> SceneManager::existingScenes;

static struct Init
{
    Init()
    {
        SceneManager::createScene();
    }
} init;
