#include <memory>

#include <Components/RectTransform.h>
#include <Components/ScalableVectorGraphic.h>
#include <Components/Text.h>
#include <Core/CoreEngine.h>
#include <Core/Debug.h>
#include <Core/PackageManager.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <Friends/FilledPathRenderer.h>
#include <GL/Renderer.h>
#include <PackageSystem/ExtensibleMarkupFile.h>
#include <PackageSystem/PortableNetworkGraphicFile.h>
#include <PackageSystem/TrueTypeFontFile.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;
using namespace Firework::PackageSystem;

namespace
{
    static struct ComponentStaticInit
    {
        ComponentStaticInit()
        {
            PackageManager::addBinaryFileHandler<PortableGraphicPackageFile>({ 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a });
            PackageManager::addBinaryFileHandler<TrueTypeFontPackageFile>({ 0x00, 0x01, 0x00, 0x00, 0x00 });
            PackageManager::addBinaryFileHandler<TrueTypeFontPackageFile>({ 0x74, 0x72, 0x75, 0x65, 0x00 });

            PackageManager::addTextFileHandler<ExtensibleMarkupPackageFile>(L".svg");

            InternalEngineEvent::OnRenderShutdown += []
            {
                Text::characterPaths.clear();
            };

            InternalEngineEvent::OnRenderOffloadForComponent += [](std::type_index typeIndex, Entity&, std::shared_ptr<void> component, ssz renderIndex)
            {
                if (typeIndex == typeid(Text))
                    std::static_pointer_cast<Text>(component)->renderOffload(renderIndex);
                else if (typeIndex == typeid(ScalableVectorGraphic))
                    std::static_pointer_cast<ScalableVectorGraphic>(component)->renderOffload(renderIndex);
            };

            CoreEngine::queueRenderJobForFrame([]
            {
                if (!FilledPathRenderer::renderInitialize()) [[unlikely]]
                    Debug::logError("`FilledPathRenderer` failed to render initialize.");
            });
        }
    } init;
} // namespace
