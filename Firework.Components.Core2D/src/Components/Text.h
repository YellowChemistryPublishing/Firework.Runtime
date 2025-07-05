#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <Components/RectTransform.h>
#include <Core/CoreEngine.h>
#include <Friends/FilledPathRenderer.h>
#include <Friends/PackageFileCore2D.h>
#include <GL/Transform.h>
#include <Library/Property.h>

template <typename T, typename U>
inline size_t _hash_combine(const T& t, const U& u)
{
    return std::hash<T>()(t) ^ (std::hash<U>()(u) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

namespace Firework
{
    class __firework_componentcore2d_api [[fw::component]] Text final
    {
        struct FontCharacterQuery
        {
            // Must not dereference, may outlive underlying value.
            PackageSystem::TrueTypeFontPackageFile* file;
            char32_t c;
        };

        // Main thread only.
        static robin_hood::unordered_map<FontCharacterQuery, std::shared_ptr<std::vector<FilledPathRenderer>>> characterPaths;

        std::shared_ptr<RectTransform> rectTransform;
        PackageSystem::TrueTypeFontPackageFile* _font = nullptr;
        std::u32string _text = U"";

        struct RenderData
        {
            std::vector<std::pair<std::shared_ptr<std::vector<FilledPathRenderer>>, GL::RenderTransform>> toRender;
            std::mutex toRenderLock;
        };
        std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();

        void setFont(PackageSystem::TrueTypeFontPackageFile* value)
        {
            if (this->_font == value) [[unlikely]]
                return;

            std::vector<std::pair<std::shared_ptr<std::vector<FilledPathRenderer>>, GL::RenderTransform>> swapToRender;
            for (char32_t c : this->_text)
            {
                auto charPathIt = Text::characterPaths.find(FontCharacterQuery { .file = value, .c = c });
                if (charPathIt != Text::characterPaths.end())
                {
                    swapToRender.emplace_back(std::make_pair(std::move(charPathIt->second), GL::RenderTransform()));
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
                Text::characterPaths.emplace(FontCharacterQuery { .file = value, .c = c }, pathRenderers);
                swapToRender.emplace_back(std::make_pair(std::move(pathRenderers), GL::RenderTransform()));
            }

            this->_font = value;
        }

        void renderOffload(sz renderIndex);
    public:
        Property<PackageSystem::TrueTypeFontPackageFile*, PackageSystem::TrueTypeFontPackageFile*> font { [this]() -> PackageSystem::TrueTypeFontPackageFile*
        { return this->_font; }, [this](PackageSystem::TrueTypeFontPackageFile* value)
        {
            this->setFont(value);
        } };

        friend struct std::hash<FontCharacterQuery>;
    };
} // namespace Firework

namespace std
{
    template <>
    struct hash<Firework::Text::FontCharacterQuery>
    {
        inline size_t operator()(const Firework::Text::FontCharacterQuery& value)
        {
            return _hash_combine(value.c, value.file);
        }
    };
} // namespace std
