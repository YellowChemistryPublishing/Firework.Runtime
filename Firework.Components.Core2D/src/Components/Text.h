#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <glm/vec4.hpp>

#include <Core/CoreEngine.h>
#include <EntityComponentSystem/Entity.h>
#include <Friends/Color.h>
#include <Friends/FringeRenderer.h>
#include <Friends/ShapeRenderer.h>
#include <Library/Property.h>

namespace
{
    struct ComponentStaticInit;
}

namespace Firework::PackageSystem
{
    class TrueTypeFontPackageFile;
}

namespace Firework
{
    class Entity;
    class RectTransform;

    class _fw_cc2d_api [[fw::component]] Text final
    {
        struct FontCharacterQuery
        {
            // Must not dereference, may outlive underlying value.
            PackageSystem::TrueTypeFontPackageFile* file;
            char32_t c;

            friend bool operator==(const FontCharacterQuery&, const FontCharacterQuery&) = default;
        };
        struct FontCharacterQueryHash
        {
            size_t operator()(const FontCharacterQuery& value) const
            {
                return sys::dhc2(value.c, value.file);
            }
        };

        // Main thread only.
        static robin_hood::unordered_map<FontCharacterQuery, std::shared_ptr<std::pair<ShapeRenderer, FringeRenderer>>, FontCharacterQueryHash> characterPaths;

        std::shared_ptr<RectTransform> rectTransform = nullptr;

        std::shared_ptr<PackageSystem::TrueTypeFontPackageFile> _font = nullptr;
        float _fontSize = 11.0f;
        std::u32string _text = U"";
        Color _color = Color::unknown;

        bool dirty = false;
        PackageSystem::TrueTypeFontPackageFile* deferOldFont = nullptr;
        std::u32string deferOldText = U"";

        struct RenderData
        {
            std::vector<std::pair<std::shared_ptr<std::pair<ShapeRenderer, FringeRenderer>>, std::pair<glm::mat4, glm::mat4>>> toRender;
            std::mutex toRenderLock;
        };
        std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();

        void onAttach(Entity& entity);

        std::shared_ptr<std::pair<ShapeRenderer, FringeRenderer>> findOrCreateGlyphPath(char32_t c);
        void tryBuryOrphanedGlyphPathSixFeetUnder(FontCharacterQuery q);
        void swapRenderBuffers();

        void renderOffload(ssz renderIndex);
    public:
        const Property<std::shared_ptr<PackageSystem::TrueTypeFontPackageFile>, std::shared_ptr<PackageSystem::TrueTypeFontPackageFile>> font {
            [this]() -> std::shared_ptr<PackageSystem::TrueTypeFontPackageFile> { return this->_font; },
            [this](std::shared_ptr<PackageSystem::TrueTypeFontPackageFile> value)
        {
            _fence_value_return(void(), this->_font == value);

            this->dirty = true;
            this->_font = std::move(value);
        }
        };
        const Property<float, float> fontSize { [this]() -> float { return this->_fontSize; }, [this](float value) -> void
        {
            _fence_value_return(void(), this->_fontSize == value);

            this->dirty = true;
            this->_fontSize = value;
        } };

        const Property<std::u32string, std::u32string> text { [this]() -> const std::u32string& { return this->_text; }, [this](std::u32string value)
        {
            this->dirty = true;
            this->_text = std::move(value);
        } };
        const Property<Color, const Color&> color { [this]() -> Color { return this->_color; }, [this](const Color& value)
        {
            this->dirty = true;
            this->_color = value;
        } };

        friend struct ::ComponentStaticInit;
        friend class Firework::Entity;
    };
} // namespace Firework
