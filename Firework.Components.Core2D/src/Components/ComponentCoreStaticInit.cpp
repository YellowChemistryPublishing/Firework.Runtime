#include <Components/RectTransform.h>
#include <Core/CoreEngine.h>
#include <Core/Debug.h>
#include <Core/PackageManager.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <Friends/FilledPathRenderer.h>
#include <Friends/PackageFileCore2D.h>
#include <GL/Renderer.h>
#include <Library/TypeInfo.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;
using namespace Firework::PackageSystem;

namespace Firework::Internal
{
    static struct ComponentCore2DStaticInit
    {
        ComponentCore2DStaticInit()
        {
            PackageManager::addBinaryFileHandler<PortableGraphicPackageFile>({ 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a });
            PackageManager::addBinaryFileHandler<TrueTypeFontPackageFile>({ 0x00, 0x01, 0x00, 0x00, 0x00 });
            PackageManager::addBinaryFileHandler<TrueTypeFontPackageFile>({ 0x74, 0x72, 0x75, 0x65, 0x00 });

            CoreEngine::queueRenderJobForFrame([]
            {
                if (!FilledPathRenderer::renderInitialize()) [[unlikely]]
                    Debug::logError("`FilledPathRenderer` failed to render initialize.");
            });
        }
    } init;
} // namespace Firework::Internal
