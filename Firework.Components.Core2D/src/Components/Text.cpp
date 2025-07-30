#include "Text.h"

#include <glm/gtc/quaternion.hpp>

#include <Components/RectTransform.h>
#include <Font/Font.h>
#include <Friends/ParagraphIterator.h>
#include <Friends/VectorTools.h>
#include <PackageSystem/TrueTypeFontFile.h>

using namespace Firework;
using namespace Firework::GL;
using namespace Firework::Internal;
using namespace Firework::PackageSystem;
using namespace Firework::Typography;

robin_hood::unordered_map<Text::FontCharacterQuery, std::shared_ptr<std::pair<ShapeRenderer, FringeRenderer>>, Text::FontCharacterQueryHash> Text::characterPaths;

void Text::onAttach(Entity& entity)
{
    this->rectTransform = entity.getOrAddComponent<RectTransform>();
}

std::shared_ptr<std::pair<ShapeRenderer, FringeRenderer>> Text::findOrCreateGlyphPath(char32_t c)
{
    auto charPathIt = Text::characterPaths.find(FontCharacterQuery { .file = this->_font.get(), .c = c });
    _fence_value_return(charPathIt->second, charPathIt != Text::characterPaths.end());

    const Font& f = this->_font->fontHandle();
    int glyphIndex = f.getGlyphIndex(c);
    GlyphOutline go = f.getGlyphOutline(glyphIndex);

    std::vector<ShapePoint> shapePoints;
    std::vector<ssz> shapePaths;
    float alternatePtCtrl = 1.0f;

    std::vector<FringePoint> fringePoints;
    std::vector<ssz> fringePaths;
    std::vector<glm::vec2> thisFringe;

    for (sys::integer<int> i = 0; i < go.vertsSize; i++)
    {
        switch (go.verts[+i].type)
        {
        case STBTT_vmove:
            shapePaths.emplace_back(ssz(shapePoints.size()));
            fringePaths.emplace_back(ssz(fringePoints.size()));
            [[fallthrough]];
        case STBTT_vline:
            shapePoints.emplace_back(ShapePoint { .x = float(go.verts[+i].x), .y = float(go.verts[+i].y), .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });
            fringePoints.emplace_back(FringePoint { .x = float(go.verts[+i].x), .y = float(go.verts[+i].y) });
            break;
        case STBTT_vcurve:
            {
                shapePoints.emplace_back(ShapePoint { .x = float(go.verts[+i].cx), .y = float(go.verts[+i].cy), .xCtrl = 0.0f, .yCtrl = -1.0f });
                shapePoints.emplace_back(ShapePoint { .x = float(go.verts[+i].x), .y = float(go.verts[+i].y), .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });

                if (fringePoints.empty())
                    continue;

                const glm::vec2 from(fringePoints.back().x, fringePoints.back().y);
                const glm::vec2 ctrl(float(go.verts[+i].cx), float(go.verts[+i].cy));
                const glm::vec2 to(float(go.verts[+i].x), float(go.verts[+i].y));
                (void)VectorTools::quadraticBezierToLines(from, ctrl, to, 32.0f, thisFringe);
                if (!thisFringe.empty())
                    std::transform(++thisFringe.cbegin(), thisFringe.cend(), std::back_inserter(fringePoints), [](glm::vec2 v) { return FringePoint { .x = v.x, .y = v.y }; });
                thisFringe.clear();
            }
            break;
        case STBTT_vcubic:
            {
                if (shapePoints.empty())
                    break;

                VectorTools::QuadApproxCubic converted =
                    VectorTools::cubicBeizerToQuadratic(glm::vec2(shapePoints.back().x, shapePoints.back().y), glm::vec2(float(go.verts[+i].cx), float(go.verts[+i].cy)),
                                                        glm::vec2(go.verts[+i].cx1, go.verts[+i].cy1), glm::vec2(float(go.verts[+i].x), float(go.verts[+i].y)));

                shapePoints.emplace_back(ShapePoint { .x = converted.c1.x, .y = converted.c1.y, .xCtrl = 0.0f, .yCtrl = -1.0f });
                shapePoints.emplace_back(ShapePoint { .x = converted.p2.x, .y = converted.p2.y, .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });
                shapePoints.emplace_back(ShapePoint { .x = converted.c2.x, .y = converted.c2.y, .xCtrl = 0.0f, .yCtrl = -1.0f });
                shapePoints.emplace_back(ShapePoint { .x = converted.p3.x, .y = converted.p3.y, .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });

                if (VectorTools::quadraticBezierToLines(converted.p1, converted.c1, converted.p2, 32.0f, thisFringe) && !thisFringe.empty())
                    thisFringe.pop_back();
                (void)VectorTools::quadraticBezierToLines(converted.p2, converted.c2, converted.p3, 32.0f, thisFringe);
                if (!thisFringe.empty())
                    std::transform(++thisFringe.cbegin(), thisFringe.cend(), std::back_inserter(fringePoints), [](glm::vec2 v) { return FringePoint { .x = v.x, .y = v.y }; });
                thisFringe.clear();
            }
            break;
        }
    }
    shapePaths.emplace_back(shapePoints.size());
    fringePaths.emplace_back(fringePoints.size());

    _fence_value_return(nullptr, shapePaths.size() < 2);
    _fence_value_return(nullptr, shapePoints.size() < 3);

    std::shared_ptr<std::pair<ShapeRenderer, FringeRenderer>> pathRenderers =
        std::make_shared<std::pair<ShapeRenderer, FringeRenderer>>(ShapeRenderer(shapePoints, shapePaths), FringeRenderer(fringePoints, fringePaths));

    Text::characterPaths.emplace(FontCharacterQuery { .file = this->_font.get(), .c = c }, pathRenderers);

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
    const glm::vec2& pos = this->rectTransform->position();
    const float rot = this->rectTransform->rotation();
    const glm::vec2& sc = this->rectTransform->scale();
    const Font& font = this->_font->fontHandle();
    const float glSc = this->_fontSize / float(font.height());
    const float scaledLineHeight = this->_fontSize + float(font.lineGap) * glSc;

    glm::vec2 gPos(0.0f);

    auto calcGlyphRenderTransformAndAdvance = [&](char32_t c) -> std::pair<glm::mat4, glm::mat4>
    {
        _fence_contract_enforce(this->_font != nullptr);
        _fence_contract_enforce(this->rectTransform != nullptr);

        const GlyphMetrics gm = font.getGlyphMetrics(font.getGlyphIndex(c));

        if (gPos.x != 0.0f && gPos.x + float(gm.advanceWidth) * glSc >= r.width())
        {
            gPos.x = 0;
            gPos.y -= scaledLineHeight;
        }

        glm::mat4 glTransform = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
        glTransform *= glm::mat4_cast(glm::quat(glm::vec3(0.0f, 0.0f, -rot)));
        glTransform =
            glm::translate(glTransform, glm::vec3(gPos.x + float(std::abs(gm.leftSideBearing)) * glSc + r.left * sc.x, gPos.y - float(font.ascent) * glSc + r.top * sc.y, 0.0f));
        glTransform = glm::scale(glTransform, glm::vec3(sc.x * glSc, sc.y * glSc, 0.0f));

        glm::mat4 clipTransform = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
        clipTransform *= glm::mat4_cast(glm::quat(glm::vec3(0.0f, 0.0f, -rot)));
        clipTransform = glm::translate(clipTransform, glm::vec3(gPos.x + r.left * sc.x, gPos.y - float(font.height()) * glSc + r.top * sc.y, 0.0f));
        clipTransform =
            glm::scale(clipTransform, glm::vec3(sc.x * glSc * (float(gm.advanceWidth) + float(std::abs(gm.leftSideBearing)) * 2.0f), sc.y * glSc * float(font.height()), 0.0f));
        clipTransform = glm::translate(clipTransform, glm::vec3(0.5f, 0.5f, 0.0f));

        gPos.x += float(gm.advanceWidth) * glSc;

        return std::make_pair(glTransform, clipTransform);
    };
    auto incrementLine = [&](float howMany = 1.0f)
    {
        gPos.x = 0.0f;
        gPos.y -= scaledLineHeight * howMany;
    };

    std::vector<std::pair<std::shared_ptr<std::pair<ShapeRenderer, FringeRenderer>>, std::pair<glm::mat4, glm::mat4>>> swapToRender;
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
            std::shared_ptr<std::pair<ShapeRenderer, FringeRenderer>> gPath = this->findOrCreateGlyphPath(*cIt);
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

    std::lock_guard guard(this->renderData->toRenderLock);
    std::swap(this->renderData->toRender, swapToRender);
}

void Text::renderOffload(ssz renderIndex)
{
    if (this->dirty || this->rectTransform->dirty())
    {
        if (this->_font && !this->_text.empty()) [[likely]]
            this->swapRenderBuffers();
        else
        {
            std::lock_guard guard(this->renderData->toRenderLock);
            this->renderData->toRender.clear();
        }

        if ((this->deferOldFont != this->_font.get() && this->deferOldFont) || (this->deferOldText != this->_text && !this->deferOldText.empty()))
            for (char32_t c : this->deferOldText) this->tryBuryOrphanedGlyphPathSixFeetUnder(FontCharacterQuery { .file = this->deferOldFont, .c = c });

        this->deferOldText = this->_text;
        this->deferOldFont = this->_font.get();
        this->dirty = false;
    }

    CoreEngine::queueRenderJobForFrame([renderIndex, renderData = this->renderData, rectTransform = rectTransform->matrix(), color = this->_color]
    {
        std::lock_guard guard(renderData->toRenderLock);

        for (const auto& [renderers, transforms] : renderData->toRender)
        {
            const auto& [shape, _] = *renderers;
            const auto& [glTf, clipTf] = transforms;
            (void)shape.submitDrawStencil(float(+renderIndex), glTf);
            (void)ShapeRenderer::submitDraw(float(+renderIndex), clipTf, ~0_u8, color);
        }

        for (const auto& [renderers, transforms] : renderData->toRender)
        {
            const auto& [_1, fringe] = *renderers;
            const auto& [glTf, _2] = transforms;
            (void)fringe.submitDraw(float(+renderIndex) + 0.5f, glTf, color);
        }
    }, false);
}
