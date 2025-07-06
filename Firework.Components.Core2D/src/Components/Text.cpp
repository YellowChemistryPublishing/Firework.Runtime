#include "Text.h"

using namespace Firework;
using namespace Firework::Internal;

robin_hood::unordered_map<Internal::FontCharacterQuery, std::shared_ptr<std::vector<FilledPathRenderer>>> Text::characterPaths;

void Text::renderOffload(sz renderIndex)
{
    CoreEngine::queueRenderJobForFrame([renderIndex, renderData = this->renderData, rectTransform = renderTransformFromRectTransform(rectTransform.get())]
    {
        std::lock_guard guard(renderData->toRenderLock);
        for (auto& [paths, transform] : renderData->toRender)
            for (auto& path : *paths) path.submitDrawStencil(renderIndex, transform);
        FilledPathRenderer::submitDraw(renderIndex, rectTransform);
    }, false);
}