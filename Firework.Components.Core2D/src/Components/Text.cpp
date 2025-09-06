#include "Text.h"
#include "bgfx/defines.h"

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

robin_hood::unordered_map<Text::FontCharacterQuery, std::shared_ptr<ShapeRenderer>, Text::FontCharacterQueryHash> Text::characterPaths;

void Text::onAttach(Entity& entity)
{
    this->rectTransform = entity.getOrAddComponent<RectTransform>();
}

std::shared_ptr<ShapeRenderer> Text::findOrCreateGlyphPath(char32_t c)
{
    auto charPathIt = Text::characterPaths.find(FontCharacterQuery { .file = this->_font.get(), .c = c });
    _fence_value_return(charPathIt->second, charPathIt != Text::characterPaths.end());

    const Font& f = this->_font->fontHandle();
    int glyphIndex = f.getGlyphIndex(c);
    GlyphOutline go = f.getGlyphOutline(glyphIndex);

    std::vector<ShapePoint> shapePoints;
    std::vector<uint16_t> shapeInds;
    std::vector<ShapePoint> shapeCurvePoints;
    std::vector<uint16_t> shapeCurveInds;
    std::vector<ShapeOutlinePoint> currentPath;

    const auto pushShapeData = [&]
    {
        (void)VectorTools::shapeTrianglesFromOutline(currentPath, shapePoints, shapeInds);
        (void)VectorTools::shapeProcessCurvesFromOutline(currentPath, shapeCurvePoints, shapeCurveInds, shapePoints, shapeInds);
    };

    for (sys::integer<int> i = 0; i < go.vertsSize; i++)
    {
        switch (go.verts[+i].type)
        {
        case STBTT_vmove:
            pushShapeData();
            currentPath.clear();
            [[fallthrough]];
        case STBTT_vline:
            currentPath.emplace_back(ShapeOutlinePoint { .x = float(go.verts[+i].x), .y = float(go.verts[+i].y), .isCtrl = false });
            break;
        case STBTT_vcurve:
            currentPath.emplace_back(ShapeOutlinePoint { .x = float(go.verts[+i].cx), .y = float(go.verts[+i].cy), .isCtrl = true });
            currentPath.emplace_back(ShapeOutlinePoint { .x = float(go.verts[+i].x), .y = float(go.verts[+i].y), .isCtrl = false });
            break;
        case STBTT_vcubic:
            currentPath.emplace_back(ShapeOutlinePoint { .x = float(go.verts[+i].cx), .y = float(go.verts[+i].cy), .isCtrl = true });
            currentPath.emplace_back(ShapeOutlinePoint { .x = float(go.verts[+i].cx1), .y = float(go.verts[+i].cy1), .isCtrl = true });
            currentPath.emplace_back(ShapeOutlinePoint { .x = float(go.verts[+i].x), .y = float(go.verts[+i].y), .isCtrl = false });
            break;
        }
    }
    pushShapeData();

    _fence_value_return(nullptr, shapePoints.size() < 3);
    _fence_value_return(nullptr, shapeInds.size() < 3);

    u32 curvePointsBegin = shapePoints.size();
    u32 curveIndsBegin = shapeInds.size();
    shapePoints.insert(shapePoints.end(), shapeCurvePoints.begin(), shapeCurvePoints.end());
    std::transform(shapeCurveInds.begin(), shapeCurveInds.end(), std::back_inserter(shapeInds), [curvePointsBegin](const uint16_t i) { return +(i + u16(curvePointsBegin)); });
    std::shared_ptr<ShapeRenderer> pathRenderers = std::make_shared<ShapeRenderer>(shapePoints, shapeInds, curveIndsBegin);

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
    const Font& fh = this->_font->fontHandle();
    const float glSc = this->_fontSize / float(fh.height());
    const float scaledLineHeight = this->_fontSize + float(fh.lineGap) * glSc;

    glm::vec2 gPos(0.0f);

    auto calcGlyphRenderTransformAndAdvance = [&](char32_t c) -> std::pair<glm::mat4, glm::mat4>
    {
        _fence_contract_enforce(this->_font != nullptr);
        _fence_contract_enforce(this->rectTransform != nullptr);

        const GlyphMetrics gm = fh.getGlyphMetrics(fh.getGlyphIndex(c));

        if (gPos.x != 0.0f && gPos.x + float(gm.advanceWidth) * glSc >= r.width())
        {
            gPos.x = 0;
            gPos.y -= scaledLineHeight;
        }

        glm::mat4 glTransform = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
        glTransform *= glm::mat4_cast(glm::quat(glm::vec3(0.0f, 0.0f, -rot)));
        glTransform =
            glm::translate(glTransform, glm::vec3((gPos.x + float(std::abs(gm.leftSideBearing)) * glSc + r.left) * sc.x, (gPos.y - float(fh.ascent) * glSc + r.top) * sc.y, 0.0f));
        glTransform = glm::scale(glTransform, glm::vec3(sc.x * glSc, sc.y * glSc, 0.0f));

        glm::mat4 clipTransform = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
        clipTransform *= glm::mat4_cast(glm::quat(glm::vec3(0.0f, 0.0f, -rot)));
        clipTransform = glm::translate(clipTransform, glm::vec3((gPos.x + r.left) * sc.x, (gPos.y - float(fh.height()) * glSc + r.top) * sc.y, 0.0f));
        clipTransform =
            glm::scale(clipTransform, glm::vec3(sc.x * glSc * (float(gm.advanceWidth) + float(std::abs(gm.leftSideBearing)) * 2.0f), sc.y * glSc * float(fh.height()), 0.0f));
        clipTransform = glm::translate(clipTransform, glm::vec3(0.5f, 0.5f, 0.0f));

        gPos.x += float(gm.advanceWidth) * glSc;

        return std::make_pair(glTransform, clipTransform);
    };
    auto incrementLine = [&](float howMany = 1.0f)
    {
        gPos.x = 0.0f;
        gPos.y -= scaledLineHeight * howMany;
    };

    std::vector<std::pair<std::shared_ptr<ShapeRenderer>, std::pair<glm::mat4, glm::mat4>>> swapToRender;
    for (auto wordIt = ParagraphIterator<char32_t>::begin(this->_text); wordIt != ParagraphIterator<char32_t>::end(this->_text); ++wordIt)
    {
        float wordLenScaled = std::accumulate(wordIt.textBegin(), wordIt.textEnd(), 0.0f, [&](float a, char32_t b)
        {
            GlyphMetrics gm = fh.getGlyphMetrics(fh.getGlyphIndex(b));
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
                GlyphMetrics gm = fh.getGlyphMetrics(fh.getGlyphIndex(b));
                return a + float(gm.advanceWidth);
            }
        }) * glSc;

        if (gPos.x + wordLenScaled >= r.width() && wordLenScaled < r.width())
            incrementLine();

        for (auto cIt = wordIt.textBegin(); cIt != wordIt.textEnd(); ++cIt)
        {
            std::shared_ptr<ShapeRenderer> gPath = this->findOrCreateGlyphPath(*cIt);
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

        for (const auto& [shape, transforms] : renderData->toRender)
        {
            _push_nowarn_c_cast();

            const auto& [glTf, clipTf] = transforms;
            (void)ShapeRenderer::submitDrawCover(float(+renderIndex), clipTf, 0_u8, Color(0, 0, 0, 0),
                                                 BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_ZERO) | BGFX_STATE_DEPTH_TEST_LESS, 0);
            (void)shape->submitDrawStencil(float(+renderIndex), glTf, FillRule::NonZero);
            (void)ShapeRenderer::submitDrawCover(
                float(+renderIndex), clipTf, 0_u8, color,
                BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                    BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_DST_ALPHA, BGFX_STATE_BLEND_INV_DST_ALPHA, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ZERO) |
                    BGFX_STATE_DEPTH_TEST_LESS,
                BGFX_STENCIL_TEST_NOTEQUAL | BGFX_STENCIL_OP_FAIL_S_REPLACE | BGFX_STENCIL_OP_PASS_Z_REPLACE);

            _pop_nowarn_c_cast();
        }
    }, false);
}
