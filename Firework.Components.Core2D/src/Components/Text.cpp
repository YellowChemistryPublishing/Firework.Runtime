#include "Text.h"

#include <Friends/ParagraphIterator.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Typography;
using namespace PackageSystem;
using namespace GL;

robin_hood::unordered_map<Text::FontCharacterQuery, std::shared_ptr<std::vector<FilledPathRenderer>>, Text::FontCharacterQueryHash> Text::characterPaths;

void Text::onAttach(Entity& entity)
{
    this->rectTransform = entity.getOrAddComponent<RectTransform>();
}

std::shared_ptr<std::vector<FilledPathRenderer>> Text::findOrCreateGlyphPath(char32_t c)
{
    auto charPathIt = Text::characterPaths.find(FontCharacterQuery { .file = this->_font, .c = c });
    _fence_value_return(charPathIt->second, charPathIt != Text::characterPaths.end());

    Font& font = this->_font->fontHandle();
    int glyphIndex = font.getGlyphIndex(c);
    GlyphOutline go = font.getGlyphOutline(glyphIndex);

    std::vector<size_t> spans;
    std::vector<FilledPathPoint> paths;
    for (sys::integer<int> i = 0; i < go.vertsSize; i++)
    {
        if (go.verts[+i].type == STBTT_vmove)
            spans.emplace_back(paths.size());
        paths.emplace_back(FilledPathPoint { .x = float(go.verts[+i].x), .y = float(go.verts[+i].y) });
    }
    spans.emplace_back(paths.size());

    _fence_value_return(nullptr, spans.size() <= 1);

    std::shared_ptr<std::vector<FilledPathRenderer>> pathRenderers = std::make_shared<std::vector<FilledPathRenderer>>();
    for (auto it = spans.begin(); it != --spans.end(); ++it)
    {
        size_t beg = *it;
        size_t end = *++decltype(it)(it);

        pathRenderers->emplace_back(FilledPathRenderer(std::span(paths.begin() + ptrdiff_t(beg), paths.begin() + ptrdiff_t(end))));
    }
    Text::characterPaths.emplace(FontCharacterQuery { .file = this->_font, .c = c }, pathRenderers);

    return pathRenderers;
}
void Text::tryBuryOrphanedGlyphPathSixFeetUnder(const FontCharacterQuery q)
{
    auto charPathIt = Text::characterPaths.find(q);
    _fence_value_return(void(), charPathIt == Text::characterPaths.end());

    if (charPathIt->second.use_count() <= 1)
        Text::characterPaths.erase(charPathIt);
}
void Text::swapRenderBuffers()
{
    const RectFloat& r = this->rectTransform->rect();
    const sysm::vector2& sc = this->rectTransform->scale();
    const Font& font = this->_font->fontHandle();
    const float glSc = this->_fontSize / float(font.height());
    const float scaledLineHeight = this->_fontSize + float(font.lineGap) * glSc;

    sysm::vector2 gPos = sysm::vector2::zero;

    auto calcGlyphRenderTransformAndAdvance = [&](char32_t c) -> RenderTransform
    {
        _fence_contract_enforce(this->_font);
        _fence_contract_enforce(this->rectTransform != nullptr);

        const GlyphMetrics gm = font.getGlyphMetrics(font.getGlyphIndex(c));

        if (gPos.x != 0.0f && gPos.x + float(gm.advanceWidth) * glSc >= r.width())
        {
            gPos.x = 0;
            gPos.y -= scaledLineHeight;
        }

        RenderTransform ret;
        ret.scale(sysm::vector3(sc.x * glSc, sc.y * glSc, 0));
        ret.translate(sysm::vector3(gPos.x + float(gm.leftSideBearing) * glSc + r.left * sc.x, gPos.y - float(font.ascent) * glSc + r.top * sc.y, 0));
        ret.rotate(sysm::quaternion::fromEuler(sysm::vector3(0, 0, this->rectTransform->rotation())));
        const sysm::vector2& pos = this->rectTransform->position();
        ret.translate(sysm::vector3(pos.x, pos.y, 0));

        gPos.x += float(gm.advanceWidth) * glSc;

        return ret;
    };
    auto incrementLine = [&](float howMany = 1.0f)
    {
        gPos.x = 0.0f;
        gPos.y -= scaledLineHeight * howMany;
    };

    std::vector<std::pair<std::shared_ptr<std::vector<FilledPathRenderer>>, RenderTransform>> swapToRender;
    for (auto wordIt = ParagraphIterator<char32_t>::begin(this->_text); wordIt != ParagraphIterator<char32_t>::end(this->_text); ++wordIt)
    {
        float wordLenScaled = std::accumulate(wordIt.textBegin(), wordIt.textEnd(), 0.0f, [&](float a, char32_t b)
        {
            GlyphMetrics gm = font.getGlyphMetrics(font.getGlyphIndex(b));
            return a + float(gm.advanceWidth);
        }) * glSc;
        ssz newLineCount = 0;
        float spaceLenScaled = std::accumulate(wordIt.spaceBegin(), wordIt.spaceEnd(), 0.0f, [&](float a, char32_t b)
        {
            if (b == U'\v' || b == U'\n')
            {
                ++newLineCount;
                return 0.0f;
            }
            else
            {
                GlyphMetrics gm = font.getGlyphMetrics(font.getGlyphIndex(b));
                return a + float(gm.advanceWidth);
            }
        }) * glSc;

        if (gPos.x + wordLenScaled >= r.width() && wordLenScaled < r.width())
            incrementLine();

        for (auto cIt = wordIt.textBegin(); cIt != wordIt.textEnd(); ++cIt)
        {
            std::shared_ptr<std::vector<FilledPathRenderer>> gPath = this->findOrCreateGlyphPath(*cIt);
            if (!gPath)
                continue;

            swapToRender.emplace_back(std::make_pair(std::move(gPath), calcGlyphRenderTransformAndAdvance(*cIt)));
        }

        if (newLineCount > 0)
            incrementLine(float(newLineCount));

        if (gPos.x + spaceLenScaled >= r.width())
            incrementLine();
        else
            gPos.x += spaceLenScaled;
    }

    {
        std::lock_guard guard(this->renderData->toRenderLock);
        std::swap(this->renderData->toRender, swapToRender);
    }
}

void Text::setFont(PackageSystem::TrueTypeFontPackageFile* value)
{
    _fence_value_return(void(), this->_font == value);

    if (this->_font) [[likely]]
        this->swapRenderBuffers();
    else
    {
        std::lock_guard guard(this->renderData->toRenderLock);
        this->renderData->toRender.clear();
    }

    std::swap(this->_font, value);

    if (value) [[likely]]
        for (char32_t c : this->_text) this->tryBuryOrphanedGlyphPathSixFeetUnder(FontCharacterQuery { .file = value, .c = c });
}
void Text::setFontSize(float value)
{
    this->_fontSize = value;
    _fence_value_return(void(), !this->_font);

    this->swapRenderBuffers(); // Never orphans.
}
void Text::setText(std::u32string&& value)
{
    std::swap(this->_text, value);
    _fence_value_return(void(), !this->_font);

    this->swapRenderBuffers();
    for (char32_t c : value) this->tryBuryOrphanedGlyphPathSixFeetUnder(FontCharacterQuery { .file = this->_font, .c = c });
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