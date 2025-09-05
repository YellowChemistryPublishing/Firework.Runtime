#pragma once

#include "Firework.Components.Core2D.Exports.h"

_push_nowarn_gcc(_clWarn_gcc_sign_compare);
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <module/sys>
#include <vector>
_pop_nowarn_gcc();

#include <Friends/Color.h>

namespace Firework
{
    struct ShapeOutlinePoint
    {
        float x, y;
        bool isCtrl; // Is bezier control point?
    };

    class _fw_cc2d_api VectorTools final
    {
        static void ignoreWhitespace(const char*& it, const char* end);
        static bool readFloat(const char*& it, const char* end, float& out);
    public:
        VectorTools() = delete;

        struct Viewbox
        {
            float x, y, w, h;
        };

        [[nodiscard]] static sys::result<Viewbox> parseViewbox(std::string_view attrVal);

        [[nodiscard]] static sys::result<Color> parseColor(std::string_view attrVal);

        [[nodiscard]] static sys::result<glm::mat3x3> parseTransform(std::string_view attrVal);

        struct PathCommandArcTo
        {
            glm::vec2 from;
            glm::vec2 to;
        };
        struct PathCommandClose
        {
            ssz begin, end;
        };

        enum class PathCommandType : uint_fast8_t
        {
            MoveTo,
            LineTo,
            QuadraticTo,
            CubicTo,
            ArcTo,
            ClosePath
        };
        struct PathCommand
        {
            struct ControlPointData
            {
                glm::vec2 ctrl1, ctrl2;
            };
            struct ArcData
            {
                glm::vec2 radius;
                float angleDeg;
                bool largeArcFlag, sweepFlag;
            };
            struct IndexData
            {
                ssz begin, end;
            };

            _push_nowarn_gcc(_clWarn_gcc_pedantic);
            _push_nowarn_clang(_clWarn_clang_anon_union_struct);
            _push_nowarn_clang(_clWarn_clang_nameless_struct_union);
            _push_nowarn_msvc(_clWarn_msvc_nameless_struct_union);

            glm::vec2 from;
            union
            {
                ControlPointData dc;
                ArcData da;
                IndexData di;
            };
            glm::vec2 to;
            PathCommandType type;

            _pop_nowarn_msvc();
            _pop_nowarn_clang();
            _pop_nowarn_clang();
            _pop_nowarn_gcc();

            PathCommand(glm::vec2 to) : to(to), type(PathCommandType::MoveTo)
            { }
            PathCommand(glm::vec2 from, glm::vec2 to) : from(from), to(to), type(PathCommandType::LineTo)
            { }
            PathCommand(glm::vec2 from, glm::vec2 ctrl, glm::vec2 to) : from(from), dc { .ctrl1 = glm::vec2(), .ctrl2 = ctrl }, to(to), type(PathCommandType::QuadraticTo)
            { }
            PathCommand(glm::vec2 from, glm::vec2 ctrl1, glm::vec2 ctrl2, glm::vec2 to) : from(from), dc { .ctrl1 = ctrl1, .ctrl2 = ctrl2 }, to(to), type(PathCommandType::CubicTo)
            { }
            PathCommand(glm::vec2 from, glm::vec2 to, glm::vec2 radius, float angleDeg, bool largeArcFlag, bool sweepFlag) :
                from(from), da { .radius = radius, .angleDeg = angleDeg, .largeArcFlag = largeArcFlag, .sweepFlag = sweepFlag }, to(to), type(PathCommandType::ArcTo)
            { }
            PathCommand(ssz begin, ssz end) : di { .begin = begin, .end = end }, type(PathCommandType::ClosePath)
            { }
        };

        [[nodiscard]] static bool parsePath(std::string_view attrVal, std::vector<PathCommand>& out);

        struct QuadApproxCubic
        {
            glm::vec2 p1, c1, p2, c2, p3;
        };

        [[nodiscard]] static QuadApproxCubic cubicBeizerToQuadratic(glm::vec2 p1, glm::vec2 c1, glm::vec2 c2, glm::vec2 p2);

        [[nodiscard]] static float fastQuadraticBezierLength(glm::vec2 p1, glm::vec2 c, glm::vec2 p2);
        [[nodiscard]] static bool quadraticBezierToLines(glm::vec2 p1, glm::vec2 c, glm::vec2 p2, ssz segments, std::vector<glm::vec2>& out);
        [[nodiscard]] static bool quadraticBezierToLines(glm::vec2 p1, glm::vec2 c, glm::vec2 p2, float segmentLength, std::vector<glm::vec2>& out);

        [[nodiscard]] static bool shapeTrianglesFromOutline(std::span<const ShapeOutlinePoint> points, std::vector<struct ShapePoint>& outPoints, std::vector<uint16_t>& outInds,
                                                            glm::vec2 windAround = glm::vec2(0.0f));
        [[nodiscard]] static bool shapeProcessCurvesFromOutline(std::span<const ShapeOutlinePoint> points, std::vector<struct ShapePoint>& outPoints,
                                                                std::vector<uint16_t>& outInds, std::vector<struct ShapePoint>& outTriPoints, std::vector<uint16_t>& outTriInds);
    };
} // namespace Firework
