#include "VectorTools.h"

using namespace Firework;

bool VectorTools::parse(const char* path, std::vector<VectorPathCommand>& out)
{
    _fence_contract_enforce(path);

    _fence_value_return(false, !*path);

    const char* it = path;

    auto ignoreWhitespace = [&]
    {
        while (*it && std::isspace(*it)) ++it;
    };
    ignoreWhitespace(); // Start ourselves at the first real character.

    auto readNextCommand = [&]() -> char
    {
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
        char* newIt = nullptr;
        float ret = std::strtof(it, &newIt);
        _fence_value_return(false, !newIt || it == newIt || ret == float(HUGE_VAL) || ret == float(HUGE_VALF) || ret == float(HUGE_VALL) || std::isinf(ret) || std::isnan(ret));
        it = newIt;
        out += ret;

        ignoreWhitespace();

        return true;
    };
    auto readDelimiter = [&]() -> bool
    {
        if (*it == ',' || *it == '-')
        {
            if (*it == ',')
                ++it;
            ignoreWhitespace();
            return true;
        }
        else
            return false;
    };

    ssz pathBegin = ssz(out.size());
    sysm::vector2 cur = sysm::vector2::zero;

    char injectedNextCommand = 0;
    sysm::vector2 to, c1, c2;
    float* horizReadTo;
    while (*it)
    {
        char command;
        if (injectedNextCommand)
        {
            command = injectedNextCommand;
            injectedNextCommand = 0;
        }
        else
            _fence_value_return(false, !(command = readNextCommand()));

        switch (command)
        {
        case 'M': to = sysm::vector2::zero; goto AnyMoveTo;
        case 'm':
            to = cur;
        AnyMoveTo:
            pathBegin = it - path;

            _fence_value_return(false, !readAddNextFloat(to.x));
            _fence_value_return(false, !readDelimiter());
            _fence_value_return(false, !readAddNextFloat(to.y));

            out.emplace_back(VectorPathCommand { .moveTo = VectorPathCommandMoveTo { .to = to }, .type = VectorPathCommandType::MoveTo });

            if (readDelimiter())
                injectedNextCommand = command == 'M' ? 'L' : 'l';
            break;

        case 'L': to = sysm::vector2::zero; goto AnyLineTo;
        case 'l':
            to = cur;
        AnyLineTo:
            _fence_value_return(false, !readAddNextFloat(to.x));
            _fence_value_return(false, !readDelimiter());
            _fence_value_return(false, !readAddNextFloat(to.y));

            out.emplace_back(VectorPathCommand { .lineTo = VectorPathCommandLineTo { .from = cur, .to = to }, .type = VectorPathCommandType::LineTo });

            if (readDelimiter())
                injectedNextCommand = command;
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
            out.emplace_back(VectorPathCommand { .lineTo = VectorPathCommandLineTo { .from = cur, .to = to }, .type = VectorPathCommandType::LineTo });
            if (readDelimiter())
                injectedNextCommand = command;
            break;

        case 'C': c1 = sysm::vector2::zero; [[fallthrough]];
        case 'S':
            c2 = sysm::vector2::zero;
            to = sysm::vector2::zero;
            goto AnyCubicTo;
        case 'c': c1 = cur; [[fallthrough]];
        case 's':
            c2 = cur;
            to = cur;
        AnyCubicTo:
            if (command == 'S' || command == 's')
            {
                if (!out.empty() && out.back().type == VectorPathCommandType::CubicTo)
                    c1 = cur - (out.back().cubicTo.ctrl2 - cur);
                else
                    c1 = cur;
            }
            else
            {
                _fence_value_return(false, !readAddNextFloat(c1.x));
                _fence_value_return(false, !readDelimiter());
                _fence_value_return(false, !readAddNextFloat(c1.y));
                _fence_value_return(false, !readDelimiter());
            }

            _fence_value_return(false, !readAddNextFloat(c2.x));
            _fence_value_return(false, !readDelimiter());
            _fence_value_return(false, !readAddNextFloat(c2.y));
            _fence_value_return(false, !readDelimiter());
            _fence_value_return(false, !readAddNextFloat(to.x));
            _fence_value_return(false, !readDelimiter());
            _fence_value_return(false, !readAddNextFloat(to.y));

            out.emplace_back(VectorPathCommand { .cubicTo = VectorPathCommandCubicTo { .from = cur, .ctrl1 = c1, .ctrl2 = c2, .to = to }, .type = VectorPathCommandType::CubicTo });

            if (readDelimiter())
                injectedNextCommand = command;
            break;

        case 'Q': c1 = sysm::vector2::zero; [[fallthrough]];
        case 'T': to = sysm::vector2::zero; goto AnyQuadraticTo;
        case 'q': c1 = cur; [[fallthrough]];
        case 't':
            to = cur;
        AnyQuadraticTo:
            if (command == 'T' || command == 't')
            {
                if (!out.empty() && out.back().type == VectorPathCommandType::QuadraticTo)
                    c1 = cur - (out.back().quadraticTo.ctrl - cur);
                else
                    c1 = cur;
            }
            else
            {
                _fence_value_return(false, !readAddNextFloat(c1.x));
                _fence_value_return(false, !readDelimiter());
                _fence_value_return(false, !readAddNextFloat(c1.y));
                _fence_value_return(false, !readDelimiter());
            }

            _fence_value_return(false, !readAddNextFloat(to.x));
            _fence_value_return(false, !readDelimiter());
            _fence_value_return(false, !readAddNextFloat(to.y));

            out.emplace_back(VectorPathCommand { .quadraticTo = VectorPathCommandQuadraticTo { .from = cur, .ctrl = c1, .to = to }, .type = VectorPathCommandType::QuadraticTo });

            if (readDelimiter())
                injectedNextCommand = command;
            break;

        case 'A':
        case 'a': return false; // Not even going to try.

        case 'Z':
        case 'z':
            out.emplace_back(VectorPathCommand { .closePath = VectorPathCommandClose { .begin = pathBegin, .end = ssz(out.size()) }, .type = VectorPathCommandType::ClosePath });
            pathBegin = ssz(out.size());
            break;
        }

        cur = to;
    }

    return true;
}
