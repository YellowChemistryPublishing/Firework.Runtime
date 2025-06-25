#include "EntityManagement.h"

#include <Objects/Entity2D.h>

using namespace Firework;
using namespace Firework::Internal;

robin_hood::unordered_set<void*> Entities::entities;
robin_hood::unordered_map<size_t, robin_hood::unordered_map<Entity*, std::shared_ptr<void>>> Entities::table;

robin_hood::unordered_map<uint64_t, uint64_t> EntityManager::existingComponents;
robin_hood::unordered_map<std::pair<Entity*, uint64_t>, Component*, EntityComponentHash<Entity>> EntityManager::components;