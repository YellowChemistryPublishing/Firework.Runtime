#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <module/sys.Mathematics>

#include <Components/RectTransform.h>
#include <Core/CoreEngine.h>
#include <EntityComponentSystem/Entity.h>
#include <Friends/FilledPathRenderer.h>
#include <Friends/PackageFileCore2D.h>
#include <GL/Transform.h>
#include <Library/Property.h>

template <typename T, typename U>
inline size_t _hash_combine(const T& t, const U& u)
{
    size_t seed = std::hash<T>()(t);
    return seed ^ (std::hash<U>()(u) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

namespace Firework::Internal
{
    struct FontCharacterQuery
    {
        // Must not dereference, may outlive underlying value.
        PackageSystem::TrueTypeFontPackageFile* file;
        char32_t c;

        friend inline bool operator==(const FontCharacterQuery&, const FontCharacterQuery&) noexcept = default;
    };
} // namespace Firework::Internal

namespace std
{
    template <>
    struct hash<Firework::Internal::FontCharacterQuery>
    {
        inline size_t operator()(const Firework::Internal::FontCharacterQuery& value) const
        {
            return _hash_combine(value.c, value.file);
        }
    };
} // namespace std

namespace Firework
{
    class Entity;

    class __firework_componentcore2d_api [[fw::component]] Text final
    {
        // Main thread only.
        static robin_hood::unordered_map<Internal::FontCharacterQuery, std::shared_ptr<std::vector<FilledPathRenderer>>> characterPaths;

        std::shared_ptr<RectTransform> rectTransform;
        PackageSystem::TrueTypeFontPackageFile* _font = nullptr;
        float _fontSize = 11.0f;
        std::u32string _text = U"";

        struct RenderData
        {
            std::vector<std::pair<std::shared_ptr<std::vector<FilledPathRenderer>>, GL::RenderTransform>> toRender;
            std::mutex toRenderLock;
        };
        std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();

        void onAttach(Entity& entity)
        {
            this->rectTransform = entity.getOrAddComponent<RectTransform>();
        }

        void setFont(PackageSystem::TrueTypeFontPackageFile* value)
        {
            if (this->_font == value) [[unlikely]]
                return;

            // std::vector<std::pair<std::shared_ptr<std::vector<FilledPathRenderer>>, GL::RenderTransform>> swapToRender;
            // for (char32_t c : this->_text)
            // {
            //     auto charPathIt = Text::characterPaths.find(Internal::FontCharacterQuery { .file = value, .c = c });
            //     if (charPathIt != Text::characterPaths.end())
            //     {
            //         swapToRender.emplace_back(std::make_pair(std::move(charPathIt->second), GL::RenderTransform()));
            //         continue;
            //     }

            //     Typography::Font& font = this->_font->fontHandle();
            //     int glyphIndex = font.getGlyphIndex(c);
            //     Typography::GlyphOutline go = font.getGlyphOutline(glyphIndex);

            //     std::vector<size_t> spans;
            //     std::vector<FilledPathPoint> paths;
            //     for (sys::integer<int> i = 0; i < go.vertsSize; i++)
            //     {
            //         if (go.verts[+i].type == STBTT_vmove)
            //             spans.emplace_back(paths.size());
            //         paths.emplace_back(FilledPathPoint { .x = float(go.verts[+i].x), .y = float(go.verts[+i].y) });
            //     }
            //     spans.emplace_back(paths.size());

            //     if (spans.size() <= 1)
            //         continue;

            //     std::shared_ptr<std::vector<FilledPathRenderer>> pathRenderers = std::make_shared<std::vector<FilledPathRenderer>>();
            //     for (auto it = spans.begin(); it != --spans.end(); ++it)
            //     {
            //         size_t beg = *it;
            //         size_t end = *++decltype(it)(it);

            //         pathRenderers->emplace_back(FilledPathRenderer(std::span(paths.begin() + beg, paths.begin() + end)));
            //     }
            //     Text::characterPaths.emplace(Internal::FontCharacterQuery { .file = value, .c = c }, pathRenderers);
            //     swapToRender.emplace_back(std::make_pair(std::move(pathRenderers), GL::RenderTransform()));
            // }

            this->_font = value;
        }

        void setText(std::u32string&& value)
        {
            const RectFloat& r = this->rectTransform->rect();
            const sysm::vector2& sc = this->rectTransform->scale();
            const Typography::Font& font = this->_font->fontHandle();
            const float glSc = this->_fontSize / float(font.height());

            sysm::vector2 gPos = sysm::vector2::zero;

            auto calcGlyphRenderTransformAndAdvance = [&](char32_t c) -> GL::RenderTransform
            {
                _fence_contract_enforce(this->_font);
                _fence_contract_enforce(this->rectTransform != nullptr);

                const Typography::GlyphMetrics gm = font.getGlyphMetrics(font.getGlyphIndex(c));

                GL::RenderTransform ret;
                ret.scale(sysm::vector3(sc.x * glSc, sc.y * glSc, 0));
                ret.translate(sysm::vector3(gPos.x + gm.leftSideBearing * glSc + r.left * sc.x, gPos.y - font.ascent * glSc + r.top * sc.y, 0));
                ret.rotate(sysm::quaternion::fromEuler(sysm::vector3(0, 0, this->rectTransform->rotation())));
                const sysm::vector2& pos = this->rectTransform->position();
                ret.translate(sysm::vector3(pos.x, pos.y, 0));

                gPos.x += gm.advanceWidth * glSc;
                if (gPos.x >= r.width())
                {
                    gPos.x = 0;
                    gPos.y -= this->_fontSize + font.lineGap * glSc;
                }

                return ret;
            };

            std::vector<std::pair<std::shared_ptr<std::vector<FilledPathRenderer>>, GL::RenderTransform>> swapToRender;
            for (char32_t c : value)
            {
                auto charPathIt = Text::characterPaths.find(Internal::FontCharacterQuery { .file = this->_font, .c = c });
                if (charPathIt != Text::characterPaths.end())
                {
                    swapToRender.emplace_back(std::make_pair(std::move(charPathIt->second), calcGlyphRenderTransformAndAdvance(c)));
                    continue;
                }

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
                    continue;

                std::shared_ptr<std::vector<FilledPathRenderer>> pathRenderers = std::make_shared<std::vector<FilledPathRenderer>>();
                for (auto it = spans.begin(); it != --spans.end(); ++it)
                {
                    size_t beg = *it;
                    size_t end = *++decltype(it)(it);

                    pathRenderers->emplace_back(FilledPathRenderer(std::span(paths.begin() + beg, paths.begin() + end)));
                }
                Text::characterPaths.emplace(Internal::FontCharacterQuery { .file = this->_font, .c = c }, pathRenderers);
                swapToRender.emplace_back(std::make_pair(std::move(pathRenderers), calcGlyphRenderTransformAndAdvance(c)));
            }

            this->_text = value;

            {
                std::lock_guard guard(this->renderData->toRenderLock);
                this->renderData->toRender = std::move(swapToRender);
            }
        }

        void renderOffload(sz renderIndex);
    public:
        Property<PackageSystem::TrueTypeFontPackageFile*, PackageSystem::TrueTypeFontPackageFile*> font { [this]() -> PackageSystem::TrueTypeFontPackageFile*
        { return this->_font; }, [this](PackageSystem::TrueTypeFontPackageFile* value)
        {
            this->setFont(value);
        } };
        Property<float, float> fontSize { [this]() -> float { return this->_fontSize; }, [this](float value) -> void
        {
            this->_fontSize = value;
        } };

        Property<std::u32string, std::u32string> text { [this]() -> const std::u32string& { return this->_text; }, [this](std::u32string value)
        {
            this->setText(std::move(value));
        } };

        ~Text();

        friend struct std::hash<Internal::FontCharacterQuery>;

        friend class Firework::Entity;
        friend struct Firework::Internal::ComponentCore2DStaticInit;
    };
} // namespace Firework
