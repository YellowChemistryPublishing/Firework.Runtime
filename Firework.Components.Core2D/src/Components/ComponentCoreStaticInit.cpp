#include <Components/Image.h>
#include <Components/Panel.h>
#include <Components/Text.h>
#include <Components/Button.h>
#include <Core/PackageManager.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <Library/TypeInfo.h>

using namespace Firework;
using namespace Firework::Internal;
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
                Panel::renderInitialize();
                Image::renderInitialize();
                Text::renderInitialize();
            };
            InternalEngineEvent::OnRenderOffloadForComponent2D += [](Component2D* component)
            {
                switch (component->typeIndex())
                {
                case __typeid(Panel).qualifiedNameHash():
                    static_cast<Panel*>(component)->renderOffload();
                    break;
                case __typeid(Image).qualifiedNameHash():
                    static_cast<Image*>(component)->renderOffload();
                    break;
                case __typeid(Text).qualifiedNameHash():
                    static_cast<Text*>(component)->renderOffload();
                    break;
                case __typeid(ColorHighlightButton).qualifiedNameHash():
                    static_cast<ColorHighlightButton*>(component)->renderOffload();
                    break;
                }
            };
        }
    } init;
}
