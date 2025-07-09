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

namespace Firework
{
    class Entity;

    class __firework_componentcore2d_api [[fw::component]] Text final
    {
        struct FontCharacterQuery
        {
            // Must not dereference, may outlive underlying value.
            PackageSystem::TrueTypeFontPackageFile* file;
            char32_t c;

            friend inline bool operator==(const FontCharacterQuery&, const FontCharacterQuery&) = default;
        };
        struct FontCharacterQueryHash
        {
            inline size_t operator()(const FontCharacterQuery& value) const
            {
                return sys::dhc2(value.c, value.file);
            }
        };

        // Main thread only.
        static robin_hood::unordered_map<FontCharacterQuery, std::shared_ptr<std::vector<FilledPathRenderer>>, FontCharacterQueryHash> characterPaths;

        std::shared_ptr<RectTransform> rectTransform = nullptr;

        PackageSystem::TrueTypeFontPackageFile* _font = nullptr;
        float _fontSize = 11.0f;
        std::u32string _text = U"";

        struct RenderData
        {
            std::vector<std::pair<std::shared_ptr<std::vector<FilledPathRenderer>>, GL::RenderTransform>> toRender;
            std::mutex toRenderLock;
        };
        std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();

        void onAttach(Entity& entity);

        std::shared_ptr<std::vector<FilledPathRenderer>> findOrCreateGlyphPath(char32_t c);
        void tryBuryOrphanedGlyphPathSixFeetUnder(char32_t c);
        void swapRenderBuffers();

        void setFont(PackageSystem::TrueTypeFontPackageFile* value);
        void setFontSize(float value);
        void setText(std::u32string&& value);

        void renderOffload(sz renderIndex);
    public:
        Property<PackageSystem::TrueTypeFontPackageFile*, PackageSystem::TrueTypeFontPackageFile*> font { [this]() -> PackageSystem::TrueTypeFontPackageFile*
        { return this->_font; }, [this](PackageSystem::TrueTypeFontPackageFile* value)
        {
            this->setFont(value);
        } };
        Property<float, float> fontSize { [this]() -> float { return this->_fontSize; }, [this](float value) -> void
        {
            this->setFontSize(value);
        } };

        Property<std::u32string, std::u32string> text { [this]() -> const std::u32string& { return this->_text; }, [this](std::u32string value)
        {
            this->setText(std::move(value));
        } };

        ~Text();

        friend class Firework::Entity;
        friend struct Firework::Internal::ComponentCore2DStaticInit;
    };
} // namespace Firework
