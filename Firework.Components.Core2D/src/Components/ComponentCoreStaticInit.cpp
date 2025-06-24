#include <Components/Image.h>
#include <Components/Mask.h>
#include <Components/Panel.h>
#include <Components/Text.h>
#include <Core/PackageManager.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <Library/TypeInfo.h>

#include <Components/RectTransform.h>
#include <Core/CoreEngine.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <GL/Renderer.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;
using namespace Firework::PackageSystem;

namespace Firework::Internal
{
    static struct ComponentCoreStaticInit
    {
        ComponentCoreStaticInit()
        {
            PackageManager::addBinaryFileHandler<PortableGraphicPackageFile>({ 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a });
            PackageManager::addBinaryFileHandler<TrueTypeFontPackageFile>({ 0x00, 0x01, 0x00, 0x00, 0x00 });
            PackageManager::addBinaryFileHandler<TrueTypeFontPackageFile>({ 0x74, 0x72, 0x75, 0x65, 0x00 });

            EngineEvent::OnInitialize += []
            {
                Mask::renderInitialize();
                Panel::renderInitialize();
                Image::renderInitialize();
                Text::renderInitialize();
            };
            EngineEvent::OnTick += []
            {
                CoreEngine::queueRenderJobForFrame([] { Mask::currentRenderMaskValue = 0; });
            };
            InternalEngineEvent::OnRenderOffloadForComponent2D += [](Component2D* component)
            {
                switch (component->typeIndex())
                {
                case __typeid(Mask).qualifiedNameHash(): static_cast<Mask*>(component)->renderOffload(); break;
                case __typeid(Panel).qualifiedNameHash(): static_cast<Panel*>(component)->renderOffload(); break;
                case __typeid(Image).qualifiedNameHash(): static_cast<Image*>(component)->renderOffload(); break;
                case __typeid(Text).qualifiedNameHash(): static_cast<Text*>(component)->renderOffload(); break;
                }
            };
            InternalEngineEvent::OnLateRenderOffloadForComponent2D += [](Component2D* component)
            {
                switch (component->typeIndex())
                {
                case __typeid(Mask).qualifiedNameHash(): static_cast<Mask*>(component)->lateRenderOffload(); break;
                }
            };
        }
    } init;
} // namespace Firework::Internal
