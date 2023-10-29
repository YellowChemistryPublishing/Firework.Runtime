#include "Text.h"

#include <cmath>
#include <numeric>
#include <tuple>
#include <Mathematics.h>
#include <Core/CoreEngine.h>
#include <Core/Debug.h>
#include <Components/RectTransform.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <GL/Renderer.h>

#include <Text.vfAll.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::Mathematics;
using namespace Firework::Typography;
using namespace Firework::GL;
using namespace Firework::PackageSystem;

GeometryProgramHandle Text::program;
robin_hood::unordered_map<PackageSystem::TrueTypeFontPackageFile*, Text::RenderData*> Text::textFonts;

inline static float getAlignOffsetJustify(float endDiff, float mul)
{
    return endDiff * mul;
}
inline static float getAlignOffsetMajor(float endDiff, float)
{
    return endDiff;
}
inline static float getAlignOffsetCenter(float endDiff, float)
{
    return endDiff / 2.0f;
}
inline static float getAlignOffsetMinor(float, float)
{
    return 0.0f;
}

void Text::RenderData::trackCharacters(const std::vector<CharacterData>& text)
{
    for (auto it = text.begin(); it != text.end(); ++it)
    {
        auto c = this->characters.find(it->glyphIndex);
        if (c != this->characters.end())
            ++c->second->accessCount;
        else
        {
            GlyphOutline glyph = this->file->fontHandle().getGlyphOutline(it->glyphIndex);
            if (glyph.verts) [[likely]]
            {
                struct CharacterVertex
                {
                    float x = 0.0f, y = 0.0f, z = 1.0f;
                    uint32_t abgr = 0xffffffff;
                };

                auto buffers = glyph.createGeometryBuffers<CharacterVertex>();
                auto charData = this->characters.insert(robin_hood::pair<int, CharacterRenderData*>(it->glyphIndex, new CharacterRenderData { 1, { } })).first->second;
                CoreEngine::queueRenderJobForFrame([vertsFlattened = std::move(buffers.first), indsFlattened = std::move(buffers.second), data = charData]
                {
                    data->internalMesh = StaticMeshHandle::create
                    (
                        vertsFlattened.data(), vertsFlattened.size() * sizeof(CharacterVertex),
                        VertexLayout::create
                        ({
                            VertexDescriptor { bgfx::Attrib::Position, bgfx::AttribType::Float, 3 },
                            VertexDescriptor { bgfx::Attrib::Color0, bgfx::AttribType::Uint8, 4, true }
                        }),
                        indsFlattened.data(), indsFlattened.size() * sizeof(uint16_t)
                    );
                });
            }
        }
    }
}
void Text::RenderData::untrackCharacters(const std::vector<CharacterData>& text)
{
    for (auto it = text.begin(); it != text.end(); ++it)
    {
        auto charData = this->characters.find(it->glyphIndex);
        if (charData != this->characters.end())
        {
            --charData->second->accessCount;
            if (charData->second->accessCount == 0)
            {
                CoreEngine::queueRenderJobForFrame([data = charData->second]
                {
                    data->internalMesh.destroy();
                    delete data;
                });
                this->characters.erase(charData);
            }
        }
    }
}

Text::~Text()
{
    if (this->data) [[likely]]
    {
        this->data->untrackCharacters(this->textData);
        --this->data->accessCount;
        if (this->data->accessCount == 0)
        {
            Text::textFonts.erase(this->data->file);
            delete this->data;
        }
    }
}

void Text::setFontFile(TrueTypeFontPackageFile* value)
{
    if (this->data) [[likely]]
    {
        this->data->untrackCharacters(this->textData);
        --this->data->accessCount;
        if (this->data->accessCount == 0)
        {
            Text::textFonts.erase(this->data->file);
            delete this->data;
        }
    }
    if (value)
    {
        auto set = Text::textFonts.find(value);
        if (set != Text::textFonts.end())
        {
            this->data = set->second;
            ++this->data->accessCount;
        }
        else
        {
            this->data = Text::textFonts.insert(robin_hood::pair<TrueTypeFontPackageFile*, RenderData*>(value, new RenderData { 1, { }, value })).first->second;
            this->data->trackCharacters(this->textData);
        }
    }
    else this->data = nullptr;
}
void Text::setText(std::u32string value)
{
    if (this->data) [[likely]]
    {
        this->_text = std::move(value);
        std::vector<CharacterData> oldTextData = std::move(this->textData);

        this->textData.clear();
        this->textData.reserve(this->_text.size());
        Typography::Font& font = this->data->file->fontHandle();
        for (auto it = this->_text.begin(); it != this->_text.end(); ++it)
        {
            int glyphIndex = font.getGlyphIndex(*it);
            this->textData.push_back
            ({
                glyphIndex,
                font.getGlyphMetrics(glyphIndex)
            });
        }

        this->data->trackCharacters(this->textData);
        this->data->untrackCharacters(oldTextData);
    }
}
void Text::setHorizontalAlign(TextAlign value)
{
    switch (value)
    {
    case TextAlign::Justify:
        this->getHorizontalAlignOffset = getAlignOffsetJustify;
        break;
    case TextAlign::Major:
        this->getHorizontalAlignOffset = getAlignOffsetMajor;
        break;
    case TextAlign::Center:
        this->getHorizontalAlignOffset = getAlignOffsetCenter;
        break;
    case TextAlign::Minor:
        this->getHorizontalAlignOffset = getAlignOffsetMinor;
        break;
    }
}
void Text::setVerticalAlign(TextAlign value)
{
    switch (value)
    {
    case TextAlign::Justify:
        this->getVerticalAlignOffset = getAlignOffsetJustify;
        break;
    case TextAlign::Major:
        this->getVerticalAlignOffset = getAlignOffsetMajor;
        break;
    case TextAlign::Center:
        this->getVerticalAlignOffset = getAlignOffsetCenter;
        break;
    case TextAlign::Minor:
        this->getVerticalAlignOffset = getAlignOffsetMinor;
        break;
    }
}
void Text::setFontSize(uint32_t value)
{
    this->_fontSize = value;
}

uint32_t Text::calculateBestFitFontSize()
{
    if (this->data) [[likely]]
    {
        const RectTransform* const thisRect = this->rectTransform();
        Typography::Font& font = this->data->file->fontHandle();

        float accAdv = 0;
        for (auto it = this->textData.begin(); it != this->textData.end(); ++it)
            accAdv += it->metrics.advanceWidth;

        const float rectWidth = thisRect->rect().right - thisRect->rect().left;
        const float rectHeight = thisRect->rect().top - thisRect->rect().bottom;

        // Calculate optimistic font size - the largest possible with given text.
        // This is done to be generally performant with the following iterative font sizing algorithm.
        // TODO: Is it possible to eliminate the costly sqrt?
        float set = (font.ascent - font.descent) * (rectWidth / accAdv);
        uint32_t ret = set * sqrtf(rectHeight / set);

        const float asc = this->data->file->fontHandle().ascent;
        const float lineHeight = asc - this->data->file->fontHandle().descent;

        float increment = (float)ret;
        while (true)
        {
            float glyphScale = (float)ret / lineHeight;
            float lineHeightScaled = lineHeight * glyphScale;
            
            if (lineHeightScaled > rectHeight)
            {
                increment /= 2.0f;
                if (ret - (uint32_t)increment == ret)
                {
                    --ret;
                    break;
                }
                else
                {
                    ret -= increment;
                    continue;
                }
            }
            
            float lineDiff = this->data->file->fontHandle().lineGap * glyphScale + lineHeightScaled;
            float accXAdvance = 0;
            float accYAdvance = 0;

            size_t beg = 0;
            size_t end;

            // Horrific code that avoids multiple calls of find_first_*.
            // I don't even know if this was optimal.
            if
            (
                size_t _end = this->_text.find_first_of(U" \t\r\n", 0);
                _end < (end = this->_text.find_first_not_of(U" \t\r\n", 0))
            )
            {
                end = _end;
                goto HandleWord;
            }
            else goto HandleSpace;

            // Ohhhhhhh the goto abuse...
            // I am going to hell.
            // Do calculations for an individual text segment. This matters because each word cannot be split across lines, unless there is no space for it to fit in one line.
            HandleWord:
            {
                std::vector<float> advanceLengths;
                advanceLengths.reserve(end - beg);
                for (size_t i = beg; i < end; i++)
                    advanceLengths.push_back(float(this->textData[i].metrics.advanceWidth) * glyphScale);

                auto advanceLenIt = advanceLengths.begin();

                float wordLen = std::accumulate(advanceLenIt, advanceLengths.end(), 0.0f);
                if (accXAdvance + wordLen > rectWidth) [[unlikely]]
                {
                    // If the word doesn't fit in one line, split it across two, otherwise, put it on a new line.
                    if (wordLen > rectWidth) [[unlikely]]
                        goto SplitCalcWord;
                    else
                    {
                        accXAdvance = 0;
                        accYAdvance += lineDiff;
                        if (accYAdvance + lineHeightScaled > rectHeight)
                        {
                            increment /= 2.0f;
                            if (ret - (uint32_t)increment == ret)
                            {
                                --ret;
                                break;
                            }
                            else
                            {
                                ret -= increment;
                                continue;
                            }
                        }
                        goto InlineCalcWord;
                    }
                }
                else goto InlineCalcWord;

                // For when a word doesn't fit within its line, and has to be split across two.
                SplitCalcWord:
                for (auto it = advanceLengths.begin(); it != advanceLengths.end(); ++it)
                {
                    if (accXAdvance + *it > rectWidth) [[unlikely]]
                    {
                        accXAdvance = 0;
                        accYAdvance += lineDiff;
                        if (accYAdvance + lineHeightScaled > rectHeight)
                        {
                            increment /= 2.0f;
                            if (ret - (uint32_t)increment == ret)
                            {
                                --ret;
                                break;
                            }
                            else
                            {
                                ret -= increment;
                                goto Continue;
                            }
                        }
                    }
                    accXAdvance += *it;
                }
                goto ExitCalcWord;

                // For when a text segment fits within its line.
                InlineCalcWord:
                for (auto it = advanceLengths.begin(); it != advanceLengths.end(); ++it)
                    accXAdvance += *it;
                // No goto here, jump would go to the same place anyways.

                ExitCalcWord:;
            }
            beg = end;
            if (end != this->_text.size()) [[likely]]
            {
                size_t _end = this->_text.find_first_not_of(U" \t\r\n", beg + 1);
                end = (_end == std::u32string::npos ? this->_text.size() : _end);
                goto HandleSpace;
            }
            else goto ExitTextHandling;

            HandleSpace:
            for (size_t i = beg; i < end; i++)
            {
                switch (this->_text[i])
                {
                case U' ':
                    accXAdvance += this->textData[i].metrics.advanceWidth * glyphScale;
                    break;
                case U'\t':
                    accXAdvance += this->textData[i].metrics.advanceWidth * glyphScale * 8;
                    break;
                case U'\r':
                    break;
                case U'\n':
                    accXAdvance = 0;
                    accYAdvance += lineDiff;
                    if (accYAdvance + lineHeightScaled > rectHeight)
                    {
                        increment /= 2.0f;
                        if (ret - (uint32_t)increment == ret)
                        {
                            --ret;
                            goto Break;
                        }
                        else
                        {
                            ret -= increment;
                            goto Continue;
                        }
                    }
                    break;
                }
            }
            beg = end;
            if (end != this->_text.size()) [[likely]]
            {
                size_t _end = this->_text.find_first_of(U" \t\r\n", beg + 1);
                end = (_end == std::u32string::npos ? this->_text.size() : _end);
                goto HandleWord;
            }
            // No else here, it just goes to the same place anyways.

            goto ExitTextHandling;
            // More label abuse to allow for continuing and breaking
            // of the outer loop.
            Continue:
            continue;
            // Never falls through.
            Break:
            break;
            // Never falls through.
            
            ExitTextHandling:
            increment /= 2.0f;
            if (ret + (uint32_t)increment == ret)
                break;
            else ret += increment;
        }

        return ret;
    }
    else return 0;
}

void Text::renderInitialize()
{
    CoreEngine::queueRenderJobForFrame([]
    {
        switch (Renderer::rendererBackend())
        {
        #if _WIN32
        case RendererBackend::Direct3D9:
            Text::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Text, d3d9));
            break;
        case RendererBackend::Direct3D11:
            Text::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Text, d3d11));
            break;
        case RendererBackend::Direct3D12:
            Text::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Text, d3d12));
            break;
        #endif
        case RendererBackend::OpenGL:
            Text::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Text, opengl));
            break;
        case RendererBackend::Vulkan:
            Text::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Text, vulkan));
            break;
        default:
            throw "unimplemented";
        }
    });
    InternalEngineEvent::OnRenderShutdown += []
    {
        Text::program.destroy();
    };
}
void Text::renderOffload()
{
    if (this->data && !this->_text.empty()) [[likely]]
    {
        const RectTransform* const thisRect = this->rectTransform();
        const Vector2 pos = thisRect->position;
        const Vector2 scale = thisRect->scale;
        const RectFloat rect = thisRect->rect;

        const float rectWidth = (rect.right - rect.left) * scale.x;
        const float rectHeight = (rect.top - rect.bottom) * scale.y;
        const float rot = thisRect->rotation;
        const float asc = this->data->file->fontHandle().ascent;
        const float lineHeight = asc - this->data->file->fontHandle().descent;
        const float glyphScale = float(this->_fontSize) / lineHeight;
        const float lineHeightScaled = lineHeight * glyphScale * scale.y;
        const float lineDiff = (this->data->file->fontHandle().lineGap + lineHeight) * glyphScale * scale.y;

        float accYAdvance = 0.0f;
        float spaceAdv = 0.0f;

        std::vector<float> advanceLengths;
        decltype(advanceLengths)::iterator advanceLenIt;

        struct Character
        {
            CharacterRenderData* character;
            float xOffset;
        };
        struct Word
        {
            std::vector<Character> characters;
        };
        struct Line
        {
            std::vector<Word> words;
            float width;
            float yOffset;
        };
        std::vector<Line> lines
        ({{ { }, 0.0f, 0.0f }});

        auto pushCharAndIterNext = [&, this](size_t i)
        {
            auto c = this->data->characters.find(this->textData[i].glyphIndex);
            if (c != this->data->characters.end())
                lines.back().words.back().characters.push_back({ c->second, lines.back().width });

            lines.back().width += *advanceLenIt;
            ++advanceLenIt;
        };

        auto pushCurrentLine = [&]
        {
            accYAdvance += lineDiff;
            lines.push_back({ { }, 0.0f, accYAdvance });
        };
        
        size_t beg = 0;
        size_t end;

        // Horrific code that avoids multiple calls of find_first_*.
        // I don't even know if this was optimal.
        if
        (
            size_t _end = this->_text.find_first_of(U" \t\r\n", 0);
            _end < (end = this->_text.find_first_not_of(U" \t\r\n", 0))
        )
        {
            end = _end;
            goto HandleWord;
        }
        else goto HandleSpace;

        // Ohhhhhhh the goto abuse...
        // I am going to hell.
        // wait i swear im feeling some deja vu
        HandleWord:
        {
            advanceLengths.reserve(end - beg);
            for (size_t i = beg; i < end; i++)
                advanceLengths.push_back(float(this->textData[i].metrics.advanceWidth) * glyphScale * scale.x);

            advanceLenIt = advanceLengths.begin();

            float wordLen = std::accumulate(advanceLenIt, advanceLengths.end(), 0.0f);
            if (lines.back().width + spaceAdv + wordLen > rectWidth) [[unlikely]]
            {
                if (wordLen > rectWidth) [[unlikely]]
                {
                    lines.back().words.push_back({ { } });
                    lines.back().words.back().characters.reserve(end - beg);
                    goto SplitDrawWord;
                }
                else
                {
                    if (accYAdvance + lineDiff + lineHeightScaled > rectHeight)
                        goto ExitTextHandling;
                    pushCurrentLine();
                    lines.back().words.push_back({ { } });
                    lines.back().words.back().characters.reserve(end - beg);
                }
            }
            else
            {
                lines.back().width += spaceAdv;
                lines.back().words.push_back({ { } });
                lines.back().words.back().characters.reserve(end - beg);
                goto InlineDrawWord;
            }

            // Draw a word split across lines. This is used when a word is longer than the width of a line.
            SplitDrawWord:
            for (size_t i = beg; i < end; i++)
            {
                if (lines.back().width + *advanceLenIt > rectWidth) [[unlikely]]
                {
                    if (accYAdvance + lineDiff + lineHeightScaled > rectHeight)
                        goto ExitTextHandling;
                    pushCurrentLine();
                    lines.back().words.push_back({ { } });
                    lines.back().words.back().characters.reserve(end - i);
                }
                pushCharAndIterNext(i);
            }
            goto ExitDrawWord;

            // Draw a word in one line. This makes the assumption that the whole word will fit within a line.
            InlineDrawWord:
            for (size_t i = beg; i < end; i++)
                pushCharAndIterNext(i);
            // No goto here, jump would go to the same place anyways.

            ExitDrawWord:
            advanceLengths.clear();
        }
        beg = end;
        if (end != this->_text.size()) [[likely]]
        {
            size_t _end = this->_text.find_first_not_of(U" \t\r\n", beg + 1);
            end = (_end == std::u32string::npos ? this->_text.size() : _end);
            goto HandleSpace;
        }
        else goto ExitTextHandling;

        HandleSpace:
        {
            spaceAdv = 0.0f;
            for (size_t i = beg; i < end; i++)
            {
                switch (this->_text[i])
                {
                    case U' ':
                        spaceAdv += this->textData[i].metrics.advanceWidth * glyphScale * scale.x;
                        break;
                    case U'\t':
                        spaceAdv += this->textData[i].metrics.advanceWidth * glyphScale * scale.x * 8;
                        break;
                    case U'\r':
                        break;
                    case U'\n':
                    if (accYAdvance + lineDiff + lineHeightScaled > rectHeight)
                        goto ExitTextHandling;
                    spaceAdv = 0.0f;
                    pushCurrentLine();
                    break;
                }
            }
        }
        beg = end;
        if (end != this->_text.size()) [[likely]]
        {
            size_t _end = this->_text.find_first_of(U" \t\r\n", beg + 1);
            end = (_end == std::u32string::npos ? this->_text.size() : _end);
            goto HandleWord;
        }
        // No else here, it just goes to the same place anyways.

        ExitTextHandling:
        std::vector<std::pair<CharacterRenderData*, RenderTransform>> charsToDraw;
        for (auto it1 = lines.begin(); it1 != lines.end(); ++it1)
        {
            for (auto it2 = it1->words.begin(); it2 != it1->words.end(); ++it2)
            {
                charsToDraw.reserve(charsToDraw.size() + it2->characters.size());
                for (auto it3 = it2->characters.begin(); it3 != it2->characters.end(); ++it3)
                {
                    RenderTransform transform;
                    transform.scale({ glyphScale * scale.x, glyphScale * scale.y, 1.0f });
                    transform.translate
                    ({
                        rect.left * scale.x + it3->xOffset +
                        this->getHorizontalAlignOffset(rectWidth - it1->width, float(it2 - it1->words.begin()) / (it1->words.size() > 1 ? it1->words.size() - 1 : 1)),
                        (rect.top - asc * glyphScale) * scale.y - it1->yOffset -
                        this->getVerticalAlignOffset(rectHeight - lines.size() * lineDiff, float(it1 - lines.begin()) / (lines.size() > 1 ? lines.size() - 1 : 1)),
                        0.0f
                    });
                    transform.rotate(Renderer::fromEuler({ 0.0f, 0.0f, -rot }));
                    transform.translate({ pos.x, pos.y, 0.0f });
                    charsToDraw.push_back(std::make_pair(it3->character, transform));
                }
            }
        }
        CoreEngine::queueRenderJobForFrame([charsToDraw = std::move(charsToDraw)]
        {
            for (auto it = charsToDraw.begin(); it != charsToDraw.end(); ++it)
            {
                Renderer::setDrawTransform(it->second);
                Renderer::submitDraw
                (
                    1, it->first->internalMesh, Text::program,
                    BGFX_STATE_CULL_CW | BGFX_STATE_DEPTH_TEST_ALWAYS | BGFX_STATE_WRITE_Z |
                    BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A
                );
            }
        }, false);
    }
}