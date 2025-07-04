#include "Text.h"

using namespace Firework;
using namespace Firework::Internal;

robin_hood::unordered_map<Text::FontCharacterQuery, std::pair<FilledPathRenderer, sz>> Text::characterPaths;

void Text::renderOffload(sz renderIndex)
{
    CoreEngine::queueRenderJobForFrame([renderIndex, renderData = this->renderData, renderTransform = renderTransformFromRectTransform(&*this->rectTransform)]
    {
        for (auto& [c, renderer, refCount, tf] : *renderData) renderer.submitDrawStencil(renderIndex, tf);
        FilledPathRenderer::submitDraw(renderIndex, renderTransform);
    }, false);
}