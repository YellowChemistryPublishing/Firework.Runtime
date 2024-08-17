#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <robin_hood.h>
#include <Components/PackageFileCore.h>
#include <Core/PackageManager.h>
#include <GL/Renderer.h>
#include <GL/TextureVector.h>
#include <Font/Font.h>
#include <Objects/Component2D.h>
#include <Library/Property.h>

#define COMPONENT_TEXT_CHARACTER_DATA_MIN_CAPACITY 256
#define COMPONENT_TEXT_CHARACTER_DATA_OFFSETS_MIN_CAPACITY 16
#define COMPONENT_TEXT_ANTI_ALIAS 4

namespace Firework
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;

        struct CharacterPoint
        {
            float x, y;
            float xCtrl, yCtrl;
        };
    }

    enum class TextAlign : uint_fast8_t
    {
        Minor,
        Center,
        Major,
        Justify
    };

    class __firework_componentcore2d_api Text final : public Internal::Component2D
    {
        static GL::GeometryProgramHandle program;
        static GL::TextureSamplerHandle characterDataSampler;
        static GL::StaticMeshHandle unitSquare;

        static void renderInitialize();

        struct CharacterData;
        struct CharacterRenderData
        {
            uint32_t accessCount;
            size_t beg, size;
            float asc, desc, xInit, adv;
        };
        struct RenderData
        {
            // Main Thread
            uint32_t accessCount;
            PackageSystem::TrueTypeFontPackageFile* file;
            //                        v Glyph Index
            //                             v Glyph Outline
            robin_hood::unordered_map<int, std::vector<Internal::CharacterPoint>> characterData;
            //                        v Glyph Index
            //                             v Render Data
            robin_hood::unordered_map<int, CharacterRenderData*> characters;
            bool dirty = true;
            
            void trackCharacters(const std::vector<CharacterData>& text);
            void untrackCharacters(const std::vector<CharacterData>& text);
            
            // Render Thread
            robin_hood::unordered_map<const Text*, std::vector<std::pair<CharacterRenderData*, GL::RenderTransform>>> charactersForRender;

            GL::Texture2DHandle characterDataTexture;
            size_t characterDataTextureCapacity;
        };
        static robin_hood::unordered_map<PackageSystem::TrueTypeFontPackageFile*, RenderData*> textFonts;
        RenderData* data = nullptr;

        void renderOffload();

        // Rendering ^ / v Data

        struct CharacterData
        {
            int glyphIndex;
            Typography::GlyphMetrics metrics;
        };
        public: struct PositionedLine; private:

        std::u32string _text;
        std::vector<CharacterData> textData;
        std::vector<PositionedLine> _positionedText;
        bool dirty = true;

        TextAlign _horizontalAlign = TextAlign::Minor;
        TextAlign _verticalAlign = TextAlign::Minor;
        float (*getHorizontalAlignOffset)(float, float) = [](float, float) { return 0.0f; };
        float (*getVerticalAlignOffset)(float, float) = [](float, float) { return 0.0f; };

        uint32_t _fontSize = 11;

        void setFontFile(PackageSystem::TrueTypeFontPackageFile* value);
        void setFontSize(uint32_t value);
        void setText(std::u32string value);
        void setHorizontalAlign(TextAlign value);
        void setVerticalAlign(TextAlign value);

        void updateTextData();
        void recalculateCharacterPositions();
    public:
        struct PositionedCharacter
        {
            CharacterRenderData* character = nullptr;
            float xOffset = 0.0f;
            float width = 0.0f;
        };
        struct PositionedLine
        {
            std::vector<PositionedCharacter> characters;
            float width = 0.0f;
            float yOffset = 0.0f;
        };
        
        ~Text() override;

        const Property<PackageSystem::TrueTypeFontPackageFile*, PackageSystem::TrueTypeFontPackageFile*> fontFile
        {
            [this]() -> PackageSystem::TrueTypeFontPackageFile* { return this->data ? this->data->file : nullptr; },
            [this](PackageSystem::TrueTypeFontPackageFile* value) { this->setFontFile(value); }
        };
        const Property<uint32_t, uint32_t> fontSize
        {
            [&]() -> uint32_t { return this->_fontSize; },
            [&, this](uint32_t value) { this->setFontSize(value); }
        };
        const Property<const std::u32string&, std::u32string> text
        {
            [this]() -> const std::u32string& { return this->_text; },
            [this](std::u32string value) { this->setText(value); }
        };

        const std::vector<PositionedLine>& positionedText() const
        {
            return this->_positionedText;
        }
        
        const Property<TextAlign, TextAlign> horizontalAlign
        {
            [&]() -> TextAlign { return this->_horizontalAlign; },
            [&, this](TextAlign value) { this->setHorizontalAlign(value); }
        };
        const Property<TextAlign, TextAlign> verticalAlign
        {
            [&]() -> TextAlign { return this->_verticalAlign; },
            [&, this](TextAlign value) { this->setVerticalAlign(value); }
        };

        uint32_t calculateBestFitFontSize();
        float calculateBestFitHeight();

        friend struct Firework::Internal::ComponentCoreStaticInit;
    };
}