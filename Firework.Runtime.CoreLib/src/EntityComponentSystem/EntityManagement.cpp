#include "EntityManagement.h"

#include <Objects/Entity2D.h>

using namespace Firework;
using namespace Firework::Internal;

robin_hood::unordered_map<uint64_t, uint64_t> EntityManager2D::existingComponents;
std::map<std::pair<Entity2D*, uint64_t>, Component2D*> EntityManager2D::components;

robin_hood::unordered_map<uint64_t, uint64_t> EntityManager::existingComponents;
robin_hood::unordered_map<std::pair<Entity*, uint64_t>, Component*, EntityComponentHash<Entity>> EntityManager::components;