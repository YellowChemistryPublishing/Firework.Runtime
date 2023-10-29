#include "Debug.h"

#include <EntityComponentSystem/SceneManagement.h>
#include <Objects/Entity2D.h>

using namespace Firework;

SpinLock Debug::outputLock;

void Debug::printHierarchy()
{
    using namespace Firework::Internal;

    Debug::outputLock.lock();

    auto printEntity2D = [&](auto&& printEntity2D, Entity2D* entity, int depth) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            for (int i = 0; i < depth; i++)
                std::wcout << L'\t';
            std::wcout << it->name << L'\n';
            printEntity2D(printEntity2D, it, depth + 1);
        }
    };
    auto printEntity = [&](auto&& printEntity, Entity* entity, int depth) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            for (int i = 0; i < depth; i++)
                std::wcout << L'\t';
            std::wcout << it->name << L'\n';
            printEntity(printEntity, it, depth + 1);
        }
    };

    for (auto _it1 = SceneManager::existingScenes.begin(); _it1 != SceneManager::existingScenes.end(); ++_it1)
    {
        auto it1 = reinterpret_cast<Scene*>(&_it1->data);

        std::wcout << it1->_name << L"\n\t[2D]\n";
        for (auto it2 = it1->front2D; it2 != nullptr; it2 = it2->next)
        {
            std::wcout << L'\t' << it2->name << L'\n';
            printEntity2D(printEntity2D, it2, 2);
        }
        std::wcout << L"\t[3D]\n";
        for (auto it2 = it1->front; it2 != nullptr; it2 = it2->next)
        {
            std::wcout << L'\t' << it2->name << L'\n';
            printEntity(printEntity, it2, 2);
        }
    }

    Debug::outputLock.unlock();
}
