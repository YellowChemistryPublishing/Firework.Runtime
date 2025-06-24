#include <Components/Camera.h>
#include <Components/Mesh.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <Library/TypeInfo.h>

namespace Firework::Internal
{
    static struct ComponentCoreStaticInit
    {
        ComponentCoreStaticInit()
        {
            EngineEvent::OnInitialize += []()
            {
                Mesh::renderInitialize();
            };
            InternalEngineEvent::OnRenderOffloadForComponent += [](Component* component)
            {
                switch (component->typeIndex())
                {
                case __typeid(Mesh).qualifiedNameHash(): static_cast<Mesh*>(component)->renderOffload(); break;
                }
            };
            EngineEvent::OnTick += []
            {
                if (Camera::mainCamera)
                    Camera::mainCamera->project();
            };
        }
    } init;
} // namespace Firework::Internal
