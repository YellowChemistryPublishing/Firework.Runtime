#include "VectorTools.h"

using namespace Firework;

bool VectorTools::parsePath(std::string_view attrVal, std::vector<PathCommand>& out)
{
    _fence_value_return(false, attrVal.empty());

    const char* it = std::to_address(attrVal.begin());
    const char* end = std::to_address(attrVal.end());

    auto ignoreWhitespace = [&]() -> void
    {
        while (it != end && std::isspace(*it)) ++it;
    };
    ignoreWhitespace(); // Start ourselves at the first real character.

    auto readNextCommand = [&]() -> char
    {
        _fence_value_return(false, it == end);

        if (VectorTools::isCommand(*it))
        {
            char ret = *it;
            ++it;
            ignoreWhitespace();
            return ret;
        }
        else
            return 0;
    };
    auto readAddNextFloat = [&](float& out) -> bool
    {
        _fence_value_return(false, it == end);

        char* newIt = nullptr;
        float ret = std::strtof(it, &newIt);
        _fence_value_return(false, !newIt || it == newIt || ret == float(HUGE_VAL) || ret == float(HUGE_VALF) || ret == float(HUGE_VALL) || std::isinf(ret) || std::isnan(ret));
        it = newIt > end ? end : newIt;
        out += ret;

        ignoreWhitespace();

        return true;
    };
    auto readDelimiter = [&]() -> bool
    {
        _fence_value_return(false, it == end);
        _fence_value_return(false, *it != ',' && *it != '-');

        if (*it == ',')
            ++it;
        ignoreWhitespace();
        return true;
    };
    auto readAddNextFloatPair = [&](float& a, float& b) -> bool
    {
        _fence_value_return(false, !readAddNextFloat(a));
        _fence_value_return(false, !readDelimiter());
        _fence_value_return(false, !readAddNextFloat(b));
        return true;
    };

    ssz pathBegin = ssz(out.size());
    sysm::vector2 cur = sysm::vector2::zero;

    char command; // Always reinitialized in loop.
    char injectedNextCommand = 0;
    auto readNextSetInjectedCommandIfNeeded = [&](char injectAs) -> void
    {
        if (readDelimiter())
            injectedNextCommand = command;
    };

    while (it != end)
    {
        sysm::vector2 to, c1, c2;
        float* horizReadTo;

        if (injectedNextCommand)
        {
            command = injectedNextCommand;
            injectedNextCommand = 0;
        }
        else
            _fence_value_return(false, !(command = readNextCommand()));

        switch (command)
        {
        case 'M':
            to = sysm::vector2::zero;
            goto AnyMoveTo;
        case 'm':
            to = cur;
        AnyMoveTo:
            pathBegin = ssz(out.size());
            _fence_value_return(false, !readAddNextFloatPair(to.x, to.y));
            out.emplace_back(PathCommand { .moveTo = PathCommandMoveTo { .to = to }, .type = PathCommandType::MoveTo });
            readNextSetInjectedCommandIfNeeded(command == 'M' ? 'L' : 'l');
            break;

        case 'L':
            to = sysm::vector2::zero;
            goto AnyLineTo;
        case 'l':
            to = cur;
        AnyLineTo:
            _fence_value_return(false, !readAddNextFloatPair(to.x, to.y));
            out.emplace_back(PathCommand { .lineTo = PathCommandLineTo { .from = cur, .to = to }, .type = PathCommandType::LineTo });
            readNextSetInjectedCommandIfNeeded(command);
            break;

        case 'H':
            to = sysm::vector2(0.0f, cur.y);
            horizReadTo = &to.x;
            goto AnyHorizTo;
        case 'h':
            to = cur;
            horizReadTo = &to.x;
            goto AnyHorizTo;
        case 'V':
            to = sysm::vector2(cur.x, 0.0f);
            horizReadTo = &to.y;
            goto AnyHorizTo;
        case 'v':
            to = cur;
            horizReadTo = &to.y;
        AnyHorizTo:
            _fence_value_return(false, !readAddNextFloat(*horizReadTo));
            out.emplace_back(PathCommand { .lineTo = PathCommandLineTo { .from = cur, .to = to }, .type = PathCommandType::LineTo });
            readNextSetInjectedCommandIfNeeded(command);
            break;

        case 'C':
            c1 = sysm::vector2::zero;
            [[fallthrough]];
        case 'S':
            c2 = sysm::vector2::zero;
            to = sysm::vector2::zero;
            goto AnyCubicTo;
        case 'c':
            c1 = cur;
            [[fallthrough]];
        case 's':
            c2 = cur;
            to = cur;
        AnyCubicTo:
            {
                if (command == 'S' || command == 's')
                {
                    if (!out.empty() && out.back().type == PathCommandType::CubicTo)
                        c1 = cur - (out.back().cubicTo.ctrl2 - cur);
                    else
                        c1 = cur;
                }
                else
                {
                    _fence_value_return(false, !readAddNextFloatPair(c1.x, c1.y));
                    _fence_value_return(false, !readDelimiter());
                }

                _fence_value_return(false, !readAddNextFloatPair(c2.x, c2.y));
                _fence_value_return(false, !readDelimiter());
                _fence_value_return(false, !readAddNextFloatPair(to.x, to.y));

                out.emplace_back(
                    PathCommand { .cubicTo = PathCommandCubicTo { .from = cur, .ctrl1 = c1, .ctrl2 = c2, .to = to }, .type = PathCommandType::CubicTo });

                readNextSetInjectedCommandIfNeeded(command);
            }
            break;

        case 'Q':
            c1 = sysm::vector2::zero;
            [[fallthrough]];
        case 'T':
            to = sysm::vector2::zero;
            goto AnyQuadraticTo;
        case 'q':
            c1 = cur;
            [[fallthrough]];
        case 't':
            to = cur;
        AnyQuadraticTo:
            {
                if (command == 'T' || command == 't')
                {
                    if (!out.empty() && out.back().type == PathCommandType::QuadraticTo)
                        c1 = cur - (out.back().quadraticTo.ctrl - cur);
                    else
                        c1 = cur;
                }
                else
                {
                    _fence_value_return(false, !readAddNextFloatPair(c1.x, c1.y));
                    _fence_value_return(false, !readDelimiter());
                }

                _fence_value_return(false, !readAddNextFloatPair(to.x, to.y));
                out.emplace_back(
                    PathCommand { .quadraticTo = PathCommandQuadraticTo { .from = cur, .ctrl = c1, .to = to }, .type = PathCommandType::QuadraticTo });
                readNextSetInjectedCommandIfNeeded(command);
            }
            break;

        case 'A':
        case 'a':
            return false; // Not even going to try.

        case 'Z':
        case 'z':
            out.emplace_back(PathCommand { .closePath = PathCommandClose { .begin = pathBegin, .end = ssz(out.size()) }, .type = PathCommandType::ClosePath });
            pathBegin = ssz(out.size());
            break;
        }

        cur = to;
    }

    return true;
}
