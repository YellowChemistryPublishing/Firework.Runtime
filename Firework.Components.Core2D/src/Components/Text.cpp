#include "Text.h"

using namespace Firework;
using namespace Firework::Internal;

robin_hood::unordered_map<Text::FontCharacterQuery, std::shared_ptr<std::vector<FilledPathRenderer>>, Text::FontCharacterQueryHash> Text::characterPaths;

Text::~Text()
{
    CoreEngine::queueRenderJobForFrame([renderData = this->renderData] { });
}

std::shared_ptr<std::vector<FilledPathRenderer>> Text::findOrCreateGlyphPath(char32_t c)
{
    if (auto charPathIt = Text::characterPaths.find(FontCharacterQuery { .file = this->_font, .c = c }); charPathIt != Text::characterPaths.end())
        return charPathIt->second;

    Typography::Font& font = this->_font->fontHandle();
    int glyphIndex = font.getGlyphIndex(c);
    Typography::GlyphOutline go = font.getGlyphOutline(glyphIndex);

    std::vector<size_t> spans;
    std::vector<FilledPathPoint> paths;
    for (sys::integer<int> i = 0; i < go.vertsSize; i++)
    {
        if (go.verts[+i].type == STBTT_vmove)
            spans.emplace_back(paths.size());
        paths.emplace_back(FilledPathPoint { .x = float(go.verts[+i].x), .y = float(go.verts[+i].y) });
    }
    spans.emplace_back(paths.size());

    if (spans.size() <= 1)
        return nullptr;

    std::shared_ptr<std::vector<FilledPathRenderer>> pathRenderers = std::make_shared<std::vector<FilledPathRenderer>>();
    for (auto it = spans.begin(); it != --spans.end(); ++it)
    {
        size_t beg = *it;
        size_t end = *++decltype(it)(it);

        pathRenderers->emplace_back(FilledPathRenderer(std::span(paths.begin() + beg, paths.begin() + end)));
    }
    Text::characterPaths.emplace(FontCharacterQuery { .file = this->_font, .c = c }, pathRenderers);

    return pathRenderers;
}
void Text::tryBuryOrphanedGlyphPathSixFeetUnder(char32_t c)
{
    auto charPathIt = Text::characterPaths.find(FontCharacterQuery { .file = this->_font, .c = c });
    if (charPathIt == Text::characterPaths.end()) [[unlikely]]
        return;

    if (charPathIt->second.use_count() <= 1)
        Text::characterPaths.erase(charPathIt);
}

void Text::renderOffload(sz renderIndex)
{
    CoreEngine::queueRenderJobForFrame([renderIndex, renderData = this->renderData, rectTransform = renderTransformFromRectTransform(rectTransform.get())]
    {
        std::lock_guard guard(renderData->toRenderLock);
        for (auto& [paths, transform] : renderData->toRender)
            for (auto& path : *paths) (void)path.submitDrawStencil(renderIndex, transform);
        (void)FilledPathRenderer::submitDraw(renderIndex, rectTransform);
    }, false);
}