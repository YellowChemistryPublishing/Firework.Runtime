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
        static robin_hood::unordered_map<FontCharacterQuery, size_t> characterRefCounts;
        // Render thread only.
        static robin_hood::unordered_map<FontCharacterQuery, std::shared_ptr<std::vector<FilledPathRenderer>>> characterPaths;

        std::shared_ptr<RectTransform> rectTransform;
        PackageSystem::TrueTypeFontPackageFile* _font = nullptr;
        std::u32string _text = U"";

        struct CharacterRenderData
        {
            PackageSystem::TrueTypeFontPackageFile* file;
            char32_t c;
            std::shared_ptr<std::vector<FilledPathRenderer>> paths;
            GL::RenderTransform tf;

            inline CharacterRenderData(PackageSystem::TrueTypeFontPackageFile* file, char32_t c, GL::RenderTransform tf) :
                file(file), c(c), paths(
                                      [&]() -> std::shared_ptr<std::vector<FilledPathRenderer>>
            {
                auto pathIt = Text::characterPaths.find(FontCharacterQuery { file, c });
                if (pathIt != Text::characterPaths.end()) [[likely]]
                    return pathIt->second;
                else
                    return nullptr;
            }()),
                tf(std::move(tf))
            { }
            inline ~CharacterRenderData()
            {
                if (this->paths.use_count() <= 1)
                    Text::characterPaths.erase(FontCharacterQuery { this->file, this->c });
            }
        };
        std::shared_ptr<std::vector<CharacterRenderData>> renderData = std::make_shared<std::vector<CharacterRenderData>>();

        void setFont(PackageSystem::TrueTypeFontPackageFile* value)
        {
            

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
    }
} // namespace std
