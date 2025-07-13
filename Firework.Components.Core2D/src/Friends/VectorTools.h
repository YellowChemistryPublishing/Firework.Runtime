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
            case 'z': return true;
            default: return false;
            }
        }
    public:
        VectorTools() = delete;

        struct VectorPathCommandMoveTo
        {
            sysm::vector2 to;
        };
        struct VectorPathCommandLineTo
        {
            sysm::vector2 from;
            sysm::vector2 to;
        };
        struct VectorPathCommandQuadraticTo
        {
            sysm::vector2 from;
            sysm::vector2 ctrl;
            sysm::vector2 to;
        };
        struct VectorPathCommandCubicTo
        {
            sysm::vector2 from;
            sysm::vector2 ctrl1;
            sysm::vector2 ctrl2;
            sysm::vector2 to;
        };
        struct VectorPathCommandArcTo
        {
            sysm::vector2 from;
            sysm::vector2 radius;
            float angleDeg;
            bool largeArcFlag, sweepFlag;
            sysm::vector2 to;
        };
        struct VectorPathCommandClose
        {
            ssz begin, end;
        };

        enum class VectorPathCommandType : uint_fast8_t
        {
            MoveTo,
            LineTo,
            QuadraticTo,
            CubicTo,
            ArcTo,
            ClosePath
        };
        struct VectorPathCommand
        {
            union
            {
                VectorPathCommandMoveTo moveTo;
                VectorPathCommandLineTo lineTo;
                VectorPathCommandQuadraticTo quadraticTo;
                VectorPathCommandCubicTo cubicTo;
                VectorPathCommandArcTo arcTo;
                VectorPathCommandClose closePath;
            };
            VectorPathCommandType type;
        };

        static bool parse(const char* path, std::vector<VectorPathCommand>& out);
    };
} // namespace Firework
