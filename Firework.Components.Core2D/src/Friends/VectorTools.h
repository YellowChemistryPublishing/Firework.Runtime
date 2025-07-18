#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <module/sys.Mathematics>
#include <module/sys>
#include <vector>

namespace Firework
{
    class _fw_cc2d_api VectorTools final
    {
        constexpr static bool isNumeric(char c)
        {
            return ('0' <= c && c <= '9') || c == '.' || c == '-';
        }
        constexpr static bool isCommand(char c)
        {
            switch (c)
            {
            case 'M':
            case 'm':
            case 'L':
            case 'l':
            case 'H':
            case 'h':
            case 'V':
            case 'v':
            case 'C':
            case 'c':
            case 'S':
            case 's':
            case 'Q':
            case 'q':
            case 'T':
            case 't':
            case 'A':
            case 'a':
            case 'Z':
            case 'z':
                return true;
            default:
                return false;
            }
        }
    public:
        VectorTools() = delete;

        struct PathCommandMoveTo
        {
            sysm::vector2 to;
        };
        struct PathCommandLineTo
        {
            sysm::vector2 from;
            sysm::vector2 to;
        };
        struct PathCommandQuadraticTo
        {
            sysm::vector2 from;
            sysm::vector2 ctrl;
            sysm::vector2 to;
        };
        struct PathCommandCubicTo
        {
            sysm::vector2 from;
            sysm::vector2 ctrl1;
            sysm::vector2 ctrl2;
            sysm::vector2 to;
        };
        struct PathCommandArcTo
        {
            sysm::vector2 from;
            sysm::vector2 radius;
            float angleDeg;
            bool largeArcFlag, sweepFlag;
            sysm::vector2 to;
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
        };

        static bool parsePath(std::string_view attrVal, std::vector<PathCommand>& out);

        struct QuadApproxCubic
        {
            sysm::vector2 p1, c1, p2, c2, p3;
        };

        static QuadApproxCubic cubicBeizerToQuadratic(sysm::vector2 p1, sysm::vector2 c1, sysm::vector2 c2, sysm::vector2 p2);
    };
} // namespace Firework
