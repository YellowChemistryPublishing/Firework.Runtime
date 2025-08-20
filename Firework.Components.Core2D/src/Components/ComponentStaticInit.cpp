#include <memory>

#include <Components/RectTransform.h>
#include <Components/ScalableVectorGraphic.h>
#include <Components/Text.h>
#include <Core/CoreEngine.h>
#include <Core/Debug.h>
#include <Core/PackageManager.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <EntityComponentSystem/EntityManagement.h>
#include <Friends/ShapeRenderer.h>
#include <GL/Renderer.h>
#include <PackageSystem/ExtensibleMarkupFile.h>
#include <PackageSystem/PortableNetworkGraphicFile.h>
#include <PackageSystem/TrueTypeFontFile.h>

#define STB_IMAGE_IMPLEMENTATION
_push_nowarn_gcc(_clWarn_gcc_c_cast);
_push_nowarn_gcc(_clWarn_gcc_zero_as_nullptr);
_push_nowarn_clang(_clWarn_clang_zero_as_nullptr);
_push_nowarn_clang(_clWarn_clang_c_cast);
_push_nowarn_clang(_clWarn_clang_cast_align);
_push_nowarn_clang(_clWarn_clang_implicit_fallthrough);
_push_nowarn_conv_comp();
#include <stb_image.h>
_pop_nowarn_conv_comp();
_pop_nowarn_clang();
_pop_nowarn_clang();
_pop_nowarn_clang();
_pop_nowarn_clang();
_pop_nowarn_gcc();
_pop_nowarn_gcc();

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
            PackageManager::addBinaryFileHandler<TrueTypeFontPackageFile>({ 0x00, 0x01, 0x00, 0x00 });
            PackageManager::addBinaryFileHandler<TrueTypeFontPackageFile>({ 0x4F, 0x54, 0x54, 0x4F });

            PackageManager::addTextFileHandler<ExtensibleMarkupPackageFile>(L".xml");
            PackageManager::addTextFileHandler<ExtensibleMarkupPackageFile>(L".svg");

            InternalEngineEvent::OnRenderShutdown += []
            {
                Text::characterPaths.clear();
                ScalableVectorGraphic::loadedSvgs.clear();
            };

            InternalEngineEvent::OnRenderOffloadForComponent += [](std::type_index typeIndex, Entity&, std::shared_ptr<void> component, ssz renderIndex)
            {
                if (typeIndex == typeid(Text))
                    std::static_pointer_cast<Text>(component)->renderOffload(renderIndex);
                else if (typeIndex == typeid(ScalableVectorGraphic))
                    std::static_pointer_cast<ScalableVectorGraphic>(component)->renderOffload(renderIndex);
            };
            InternalEngineEvent::OnLateRenderOffloadForComponent += [](std::type_index typeIndex, Entity&, std::shared_ptr<void> component, ssz renderIndex)
            {
                if (typeIndex == typeid(ScalableVectorGraphic))
                    std::static_pointer_cast<ScalableVectorGraphic>(component)->lateRenderOffload(renderIndex);
            };

            CoreEngine::queueRenderJobForFrame([]
            {
                if (!ShapeRenderer::renderInitialize()) [[unlikely]]
                    Debug::logError("`ShapeRenderer` failed to render initialize.");
            });
        }
    } init;
} // namespace
