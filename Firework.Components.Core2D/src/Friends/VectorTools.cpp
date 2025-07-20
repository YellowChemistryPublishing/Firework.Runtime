#include "VectorTools.h"

#include <charconv>

using namespace Firework;

void VectorTools::ignoreWhitespace(const char*& it, const char* end)
{
    while (it != end && std::isspace(*it)) ++it;
}
bool VectorTools::readFloat(const char*& it, const char* end, float& out)
{
    std::from_chars_result res = std::from_chars(it, end, out);
    _fence_value_return(false, res.ec != std::errc());

    it = res.ptr;
    VectorTools::ignoreWhitespace(it, end);
    return true;
}

sys::result<VectorTools::Viewbox> VectorTools::parseViewbox(std::string_view attrVal)
{
    const char* it = std::to_address(attrVal.begin());
    const char* const end = std::to_address(attrVal.end());

    VectorTools::ignoreWhitespace(it, end); // Start ourselves at the first real character.

    VectorTools::Viewbox ret;
    float val;

    _fence_value_return(nullptr, !VectorTools::readFloat(it, end, val));
    ret.x = val;

    _fence_value_return(nullptr, !VectorTools::readFloat(it, end, val));
    ret.y = val;

    _fence_value_return(nullptr, !VectorTools::readFloat(it, end, val));
    ret.w = val;

    _fence_value_return(nullptr, !VectorTools::readFloat(it, end, val));
    ret.h = val;

    _fence_value_return(nullptr, it != end);
    return ret;
}

sys::result<Color> VectorTools::parseColor(std::string_view attrVal)
{
    _fence_value_return(nullptr, attrVal.empty());
    _fence_value_return(nullptr, attrVal.front() != '#');

    Color ret;
    ret.a = 255;

    if (attrVal.size() == 4)
    {
        byte val;
        _fence_value_return(nullptr, std::from_chars(std::to_address(attrVal.begin() + 1), std::to_address(attrVal.begin() + 2), val, 16).ec != std::errc());
        ret.r = val + val * 0xF;
        _fence_value_return(nullptr, std::from_chars(std::to_address(attrVal.begin() + 2), std::to_address(attrVal.begin() + 3), val, 16).ec != std::errc());
        ret.g = val + val * 0xF;
        _fence_value_return(nullptr, std::from_chars(std::to_address(attrVal.begin() + 3), std::to_address(attrVal.begin() + 4), val, 16).ec != std::errc());
        ret.b = val + val * 0xF;
        return ret;
    }
    else if (attrVal.size() == 7)
    {
        byte val;
        _fence_value_return(nullptr, std::from_chars(std::to_address(attrVal.begin() + 1), std::to_address(attrVal.begin() + 3), val, 16).ec != std::errc());
        ret.r = val;
        _fence_value_return(nullptr, std::from_chars(std::to_address(attrVal.begin() + 3), std::to_address(attrVal.begin() + 5), val, 16).ec != std::errc());
        ret.g = val;
        _fence_value_return(nullptr, std::from_chars(std::to_address(attrVal.begin() + 5), std::to_address(attrVal.begin() + 7), val, 16).ec != std::errc());
        ret.b = val;
        return ret;
    }
    else
        return nullptr;
}

sys::result<glm::mat3x3> VectorTools::parseTransform(std::string_view attrVal)
{
    const char* it = std::to_address(attrVal.begin());
    const char* const end = std::to_address(attrVal.end());

    VectorTools::ignoreWhitespace(it, end); // Start ourselves at the first real character.

    glm::mat3x3 ret(1.0f);
    while (it != end)
    {
        if (std::string_view(it, end).starts_with("matrix("))
        {
            it += 7;
            VectorTools::ignoreWhitespace(it, end);

            glm::mat3x3 by(1.0f);
            float val;

            _fence_value_return(nullptr, !VectorTools::readFloat(it, end, val));
            by[0][0] = val;
            _fence_value_return(nullptr, it == end || *(it++) != ',');
            VectorTools::ignoreWhitespace(it, end);

            _fence_value_return(nullptr, !VectorTools::readFloat(it, end, val));
            by[0][1] = val;
            _fence_value_return(nullptr, it == end || *(it++) != ',');
            VectorTools::ignoreWhitespace(it, end);

            _fence_value_return(nullptr, !VectorTools::readFloat(it, end, val));
            by[1][0] = val;
            _fence_value_return(nullptr, it == end || *(it++) != ',');
            VectorTools::ignoreWhitespace(it, end);

            _fence_value_return(nullptr, !VectorTools::readFloat(it, end, val));
            by[1][1] = val;
            _fence_value_return(nullptr, it == end || *(it++) != ',');
            VectorTools::ignoreWhitespace(it, end);

            _fence_value_return(nullptr, !VectorTools::readFloat(it, end, val));
            by[2][0] = val;
            _fence_value_return(nullptr, it == end || *(it++) != ',');
            VectorTools::ignoreWhitespace(it, end);

            _fence_value_return(nullptr, !VectorTools::readFloat(it, end, val));
            by[2][1] = val;
            _fence_value_return(nullptr, it == end || *(it++) != ')');
            VectorTools::ignoreWhitespace(it, end);

            ret = by * ret;
        }
    }
    return ret;
}

bool VectorTools::parsePath(std::string_view attrVal, std::vector<PathCommand>& out)
{
    _fence_value_return(false, attrVal.empty());

    constexpr auto isCommand = [](char c) -> bool
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
    };

    const char* it = std::to_address(attrVal.begin());
    const char* const end = std::to_address(attrVal.end());

    VectorTools::ignoreWhitespace(it, end); // Start ourselves at the first real character.

    auto readNextCommand = [&]() -> char
    {
        _fence_value_return(false, it == end);

        if (isCommand(*it))
        {
            char ret = *it;
            ++it;
            VectorTools::ignoreWhitespace(it, end);
            return ret;
        }
        else
            return 0;
    };
    auto readAddNextFloat = [&](float& out) -> bool
    {
        _fence_value_return(false, it == end);

        float ret = std::numeric_limits<float>::quiet_NaN();
        std::from_chars_result res = std::from_chars(it, end, ret);

        _fence_value_return(false, res.ec != std::errc() || it == res.ptr || ret == float(HUGE_VAL) || ret == float(HUGE_VALF) || ret == float(HUGE_VALL) || std::isinf(ret) || std::isnan(ret));
        it = res.ptr;
        out += ret;

        VectorTools::ignoreWhitespace(it, end);

        return true;
    };
    auto readDelimiter = [&]() -> bool
    {
        _fence_value_return(false, it == end);
        _fence_value_return(false, *it != ',' && *it != '-');

        if (*it == ',')
            ++it;
        VectorTools::ignoreWhitespace(it, end);
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
            injectedNextCommand = injectAs;
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
            out.emplace_back(PathCommand(PathCommandMoveTo { .to = to }));
            readNextSetInjectedCommandIfNeeded(command == 'M' ? 'L' : 'l');
            break;

        case 'L':
            to = sysm::vector2::zero;
            goto AnyLineTo;
        case 'l':
            to = cur;
        AnyLineTo:
            _fence_value_return(false, !readAddNextFloatPair(to.x, to.y));
            out.emplace_back(PathCommand(PathCommandLineTo { .from = cur, .to = to }));
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
            out.emplace_back(PathCommand(PathCommandLineTo { .from = cur, .to = to }));
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

                out.emplace_back(PathCommand(PathCommandCubicTo { .from = cur, .ctrl1 = c1, .ctrl2 = c2, .to = to }));

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
                out.emplace_back(PathCommand(PathCommandQuadraticTo { .from = cur, .ctrl = c1, .to = to }));
                readNextSetInjectedCommandIfNeeded(command);
            }
            break;

        case 'A':
        case 'a':
            return false; // Not even going to try.

        case 'Z':
        case 'z':
            out.emplace_back(PathCommand(PathCommandClose { .begin = pathBegin, .end = ssz(out.size()) }));
            pathBegin = ssz(out.size());
            break;
        }

        cur = to;
    }

    return true;
}

// https://www.cemyuksel.com/research/papers/quadratic_approximation_of_cubic_curves.pdf
VectorTools::QuadApproxCubic VectorTools::cubicBeizerToQuadratic(sysm::vector2 p1, sysm::vector2 c1, sysm::vector2 c2, sysm::vector2 p2)
{
    constexpr float gamma = 0.5f;

    VectorTools::QuadApproxCubic ret;
    ret.c1 = p1 + 1.5f * gamma * (c1 - p1);
    ret.c2 = p2 + 1.5f * (1.0f - gamma) * (c2 - p2);
    ret.p2 = (1.0f - gamma) * ret.c1 + gamma * ret.c2;
    ret.p1 = p1;
    ret.p3 = p2;
    return ret;
}
