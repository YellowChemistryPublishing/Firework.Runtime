#pragma once

#include "Firework.Typography.Exports.h"

#include <concepts>
#include <map>
#include <limits>
#include <tuple> // Do not reorder alphabetical, required by <mapbox/earcut.hpp>
#include <mapbox/earcut.hpp>
#include <stb_truetype.h>

#define TYPEFACE_GLYPH_INIT_POINT std::numeric_limits<float>::max()
#define TYPEFACE_GLYPH_LINE_SEGMENT_LINEAR std::numeric_limits<float>::min()

namespace mapbox::util
{
    template <std::size_t I, typename T>
    requires requires (T t) { { t.x } -> std::convertible_to<double>; { t.y } -> std::convertible_to<double>; }
    struct nth<I, T>
    {
        inline constexpr static double get(const T& t)
        {
            if constexpr (I == 0)
                return t.x;
            else if constexpr (I == 1)
                return t.y;
            else []<bool cond = false>() { static_assert(cond, "unimplemented"); }();
        };
    };
}

namespace Firework
{
    namespace Typography
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

            template <typename PointType>
            requires requires (PointType p)
            {
                { p.x } -> std::convertible_to<float>;
                { p.y } -> std::convertible_to<float>;
                { p.xCtrl } -> std::convertible_to<float>;
                { p.yCtrl } -> std::convertible_to<float>;
            }
            inline std::vector<PointType> createCurveBuffer()
            {
                std::vector<PointType> ret;
                for (int j = 0; j < this->vertsSize; j++)
                {
                    auto& p = this->verts[j];
                    switch (p.type)
                    {
                    [[unlikely]] case STBTT_vmove:
                        ret.push_back(PointType { .x = float(p.x), .y = float(p.y), .xCtrl = TYPEFACE_GLYPH_INIT_POINT, .yCtrl = TYPEFACE_GLYPH_INIT_POINT });
                        break;
                    case STBTT_vcubic:
                        // Fallthrough and produce garbage for now.
                        // TODO: Convert cubic beziers to their quadratic approximations.
                    case STBTT_vcurve:
                        ret.push_back(PointType { .x = float(p.x), .y = float(p.y), .xCtrl = float(p.cx), .yCtrl = float(p.cy) });
                        break;
                    case STBTT_vline:
                        ret.push_back(PointType { .x = float(p.x), .y = float(p.y), .xCtrl = TYPEFACE_GLYPH_LINE_SEGMENT_LINEAR, .yCtrl = TYPEFACE_GLYPH_LINE_SEGMENT_LINEAR });
                        break;
                    }
                }
                return ret;
            }
            template <typename VertType>
            requires requires (VertType v) { { v.x } -> std::convertible_to<float>; { v.y } -> std::convertible_to<float>; }
            inline std::pair<std::vector<VertType>, std::vector<uint16_t>> createGeometryBuffers()
            {
                constexpr auto pointsAreClockwise = [](stbtt_vertex* points, size_t pointsSize) -> bool
                {
                    int32_t sign = 0;
                    for (size_t i = 0; i < pointsSize - 1; i++)
                        sign += (points[i + 1].x - points[i].x) * (points[i + 1].y + points[i].y);
                    sign += (points[pointsSize - 1].x - points[0].x) * (points[pointsSize - 1].y + points[0].y);
                    return sign >= 0;
                };

                GlyphOutline& glyph = *this;

                VertType ringBegin { .x = float(glyph.verts[0].x), .y = float(glyph.verts[0].y) };
                std::vector<std::vector<std::vector<VertType>>> points { { { ringBegin } } };
                std::vector<std::vector<uint16_t>> indexes;

                size_t firstClockwiseLoopIndex = SIZE_MAX;
                size_t ringSize = 1;
                while (glyph.verts[0].x != glyph.verts[ringSize].x || glyph.verts[0].y != glyph.verts[ringSize].y)
                    ++ringSize;
                ++ringSize;
                
                if (pointsAreClockwise(glyph.verts, ringSize))
                    firstClockwiseLoopIndex = 0;
                
                uint16_t indexOffset = 0;
                for (int j = 1; j < glyph.vertsSize; j++)
                {
                    auto& p = glyph.verts[j];

                    switch (p.type)
                    {
                    [[unlikely]] case STBTT_vmove:
                        {
                            ringSize = j + 1;
                            while (glyph.verts[j].x != glyph.verts[ringSize].x || glyph.verts[j].y != glyph.verts[ringSize].y)
                                ++ringSize;
                            ringSize -= j - 1;

                            if (pointsAreClockwise(glyph.verts + j, ringSize))
                            {
                                switch (firstClockwiseLoopIndex)
                                {
                                [[likely]] case 0:
                                    break;
                                case SIZE_MAX:
                                    firstClockwiseLoopIndex = points.back().size();
                                    goto IgnoreClockwise;
                                    break;
                                default: // What a hack, no ranged cases needed. Switch statements ftw.
                                    std::iter_swap(points.back().begin(), points.back().begin() + firstClockwiseLoopIndex);
                                    firstClockwiseLoopIndex = 0;
                                }

                                auto polyIndexes = mapbox::template earcut<uint16_t>(points.back());
                                for (auto it = polyIndexes.begin(); it != polyIndexes.end(); ++it)
                                    *it += indexOffset;
                                indexes.push_back(std::move(polyIndexes));

                                for (auto it = points.back().begin(); it != points.back().end(); ++it)
                                    indexOffset += it->size();

                                points.push_back({ });
                            }
                            IgnoreClockwise:

                            points.back().push_back({ });
                            ringBegin = { (float)glyph.verts[j].x, (float)glyph.verts[j].y };
                            points.back().back().push_back(ringBegin);
                        }
                        break;
                    case STBTT_vcurve:
                        {
                            VertType begin { .x = float(glyph.verts[j - 1].x), .y = float(glyph.verts[j - 1].y) };
                            VertType control { .x = float(p.cx), .y = float(p.cy) };
                            VertType end { .x = float(p.x), .y = float(p.y) };

                            constexpr auto approxQuadBezierLen =
                            [](const VertType& begin, const VertType& control, const VertType& end) -> float
                            {
                                float d1 = fabs(begin.x - control.x) + fabs(control.x - end.x) + fabs(begin.x - end.x);
                                float d2 = fabs(begin.y - control.y) + fabs(control.y - end.y) + fabs(begin.y - end.y);
                                return sqrt((d1 * d1) + (d2 * d2));
                            };

                            #define QUADRATIC_BEZIER_SEGMENT_LENGTH 32
                            float segments = float(size_t((approxQuadBezierLen(begin, control, end) / float(QUADRATIC_BEZIER_SEGMENT_LENGTH)) + 0.5f));
                            for (float k = 1; k < segments; k++)
                            {
                                VertType p =
                                {
                                    .x =
                                    (begin.x + (control.x - begin.x) / segments * k) +
                                    ((end.x + (control.x - end.x) / segments * (segments - k)) -
                                    (begin.x + (control.x - begin.x) / segments * k)) /
                                    segments * k,
                                    .y =
                                    (begin.y + (control.y - begin.y) / segments * k) +
                                    ((end.y + (control.y - end.y) / segments * (segments - k)) -
                                    (begin.y + (control.y - begin.y) / segments * k)) /
                                    segments * k
                                };
                                points.back().back().push_back(std::move(p));
                            }
                        }
                    [[fallthrough]];
                    case STBTT_vcubic: // Can't be stuffed converting cubic beziers to their quadratic approximations right now.
                    case STBTT_vline:
                        if (p.x == ringBegin.x && p.y == ringBegin.y) [[unlikely]]
                            continue;
                        points.back().back().push_back(VertType { .x = float(p.x), .y = float(p.y) });
                        break;
                    }
                }

                if (firstClockwiseLoopIndex != 0) [[unlikely]]
                {
                    std::iter_swap(points.back().begin(), points.back().begin() + firstClockwiseLoopIndex);
                    firstClockwiseLoopIndex = 0;
                }

                auto polyIndexes = mapbox::template earcut<uint16_t>(points.back());
                for (auto it = polyIndexes.begin(); it != polyIndexes.end(); ++it)
                    *it += indexOffset;
                indexes.push_back(std::move(polyIndexes));

                std::vector<VertType> pointsFlattened;
                size_t reserveSize = 0;
                for (auto it1 = points.begin(); it1 != points.end(); ++it1)
                    for (auto it2 = it1->begin(); it2 != it1->end(); ++it2)
                        reserveSize += it2->size();
                pointsFlattened.reserve(reserveSize);
                for (auto it1 = points.begin(); it1 != points.end(); ++it1)
                    for (auto it2 = it1->begin(); it2 != it1->end(); ++it2)
                        pointsFlattened.insert(pointsFlattened.end(), std::make_move_iterator(it2->begin()), std::make_move_iterator(it2->end()));

                std::vector<uint16_t> indexesFlattened;
                reserveSize = 0;
                for (auto it = indexes.begin(); it != indexes.end(); ++it)
                    reserveSize += it->size();
                indexesFlattened.reserve(reserveSize);
                for (auto it = indexes.begin(); it != indexes.end(); ++it)
                    indexesFlattened.insert(indexesFlattened.end(), std::make_move_iterator(it->begin()), std::make_move_iterator(it->end()));

                return std::make_pair(std::move(pointsFlattened), std::move(indexesFlattened));
            }
            
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
    }
}