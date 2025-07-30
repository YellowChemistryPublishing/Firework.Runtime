#pragma once

#include "Firework.Typography.Exports.h"

#include <concepts>
#include <map>
#include <stb_truetype.h>

namespace Firework::Typography
{
    template <typename Char>
    constexpr size_t __strlen(const Char* str) noexcept
    {
        size_t ret = 0;
        while (str[ret++] != 0);
        return ret;
    }

    class Font;

    struct GlyphOutline
    {
        stbtt_vertex* verts;
        int vertsSize;

        GlyphOutline(const GlyphOutline&) = delete; // Laziness.
        inline GlyphOutline(GlyphOutline&& other) : verts(other.verts), vertsSize(other.vertsSize)
        {
            other.verts = nullptr;
            other.vertsSize = 0;
        }
        // Not in header because STBTT_free not linked in.
        _fw_typ_api ~GlyphOutline();

        friend class Firework::Typography::Font;
    private:
        inline GlyphOutline() = default;
    };
    struct GlyphMetrics
    {
        int advanceWidth, leftSideBearing;

        friend class Firework::Typography::Font;
    private:
        inline GlyphMetrics() = default;
    };

    class _fw_typ_api Font final
    {
        stbtt_fontinfo fontInfo;
    public:
        int ascent, descent, lineGap;

        Font(unsigned char* fontData);

        int height() const
        {
            return ascent - descent;
        }

        GlyphOutline getGlyphOutline(int glyphIndex) const;
        GlyphMetrics getGlyphMetrics(int glyphIndex) const;
        int getGlyphIndex(char32_t codepoint) const;
    };
} // namespace Firework::Typography
