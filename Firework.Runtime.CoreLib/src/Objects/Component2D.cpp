#include "Component2D.h"

#include <Objects/Entity2D.h>

using namespace Firework;
using namespace Firework::Internal;

Component2D::~Component2D()
{
    EntityManager2D::components.erase(std::make_pair(this->attachedEntity, this->reflection.typeID));
}
