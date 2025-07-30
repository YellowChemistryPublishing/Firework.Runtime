#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <module/sys>
#include <vector>

#include <Friends/Color.h>

namespace Firework
{
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

        struct PathCommandMoveTo
        {
            glm::vec2 to;
        };
        struct PathCommandLineTo
        {
            glm::vec2 from;
            glm::vec2 to;
        };
        struct PathCommandQuadraticTo
        {
            glm::vec2 from;
            glm::vec2 ctrl;
            glm::vec2 to;
        };
        struct PathCommandCubicTo
        {
            glm::vec2 from;
            glm::vec2 ctrl1;
            glm::vec2 ctrl2;
            glm::vec2 to;
        };
        struct PathCommandArcTo
        {
            glm::vec2 from;
            glm::vec2 radius;
            float angleDeg;
            bool largeArcFlag, sweepFlag;
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
            union
            {
                PathCommandMoveTo moveTo;
                PathCommandLineTo lineTo;
                PathCommandQuadraticTo quadraticTo;
                PathCommandCubicTo cubicTo;
                PathCommandArcTo arcTo;
                PathCommandClose closePath;
            };
            PathCommandType type;

            PathCommand(PathCommandMoveTo moveTo) : moveTo(std::move(moveTo)), type(PathCommandType::MoveTo)
            { }
            PathCommand(PathCommandLineTo lineTo) : lineTo(std::move(lineTo)), type(PathCommandType::LineTo)
            { }
            PathCommand(PathCommandQuadraticTo quadraticTo) : quadraticTo(std::move(quadraticTo)), type(PathCommandType::QuadraticTo)
            { }
            PathCommand(PathCommandCubicTo cubicTo) : cubicTo(std::move(cubicTo)), type(PathCommandType::CubicTo)
            { }
            PathCommand(PathCommandArcTo arcTo) : arcTo(std::move(arcTo)), type(PathCommandType::ArcTo)
            { }
            PathCommand(PathCommandClose closePath) : closePath(std::move(closePath)), type(PathCommandType::ClosePath)
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
    };
} // namespace Firework
