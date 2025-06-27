#include "EntityManagement.h"

#include <EntityComponentSystem/Entity.h>

using namespace Firework;
using namespace Firework::Internal;

robin_hood::unordered_map<std::type_index, robin_hood::unordered_map<Entity*, std::shared_ptr<void>>> Entities::table;
