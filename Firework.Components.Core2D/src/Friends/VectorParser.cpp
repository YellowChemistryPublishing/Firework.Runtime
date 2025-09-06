#include "VectorParser.h"

#include <charconv>

#include <Friends/CascadingStyleParser.h>
#include <Friends/ShapeRenderer.h>

using namespace Firework;

bool VectorParser::readFloat(const char*& it, const char* end, float& out)
{
    std::from_chars_result res = std::from_chars(it, end, out);
    _fence_value_return(false, res.ec != std::errc());

    it = res.ptr;
    CascadingStyleParser::ignoreWhitespace(it, end);
    return true;
}

sys::result<VectorParser::Viewbox> VectorParser::parseViewbox(std::string_view attrVal)
{
    const char* it = std::to_address(attrVal.begin());
    const char* const end = std::to_address(attrVal.end());

    CascadingStyleParser::ignoreWhitespace(it, end); // Start ourselves at the first real character.

    VectorParser::Viewbox ret;
    float val;

    _fence_value_return(nullptr, !VectorParser::readFloat(it, end, val));
    ret.x = val;

    _fence_value_return(nullptr, !VectorParser::readFloat(it, end, val));
    ret.y = val;

    _fence_value_return(nullptr, !VectorParser::readFloat(it, end, val));
    ret.w = val;

    _fence_value_return(nullptr, !VectorParser::readFloat(it, end, val));
    ret.h = val;

    _fence_value_return(nullptr, it != end);
    return ret;
}

sys::result<Color> VectorParser::parseColor(std::string_view attrVal)
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

sys::result<glm::mat3x3> VectorParser::parseTransform(std::string_view attrVal)
{
    const char* it = std::to_address(attrVal.begin());
    const char* const end = std::to_address(attrVal.end());

    CascadingStyleParser::ignoreWhitespace(it, end); // Start ourselves at the first real character.

    glm::mat3x3 ret(1.0f);
    while (it != end)
    {
        if (std::string_view(it, end).starts_with("matrix("))
        {
            it += 7;
            CascadingStyleParser::ignoreWhitespace(it, end);

            glm::mat3x3 by(1.0f);
            float val;

            _fence_value_return(nullptr, !VectorParser::readFloat(it, end, val));
            by[0][0] = val;
            _fence_value_return(nullptr, it == end || *(it++) != ',');
            CascadingStyleParser::ignoreWhitespace(it, end);

            _fence_value_return(nullptr, !VectorParser::readFloat(it, end, val));
            by[0][1] = val;
            _fence_value_return(nullptr, it == end || *(it++) != ',');
            CascadingStyleParser::ignoreWhitespace(it, end);

            _fence_value_return(nullptr, !VectorParser::readFloat(it, end, val));
            by[1][0] = val;
            _fence_value_return(nullptr, it == end || *(it++) != ',');
            CascadingStyleParser::ignoreWhitespace(it, end);

            _fence_value_return(nullptr, !VectorParser::readFloat(it, end, val));
            by[1][1] = val;
            _fence_value_return(nullptr, it == end || *(it++) != ',');
            CascadingStyleParser::ignoreWhitespace(it, end);

            _fence_value_return(nullptr, !VectorParser::readFloat(it, end, val));
            by[2][0] = val;
            _fence_value_return(nullptr, it == end || *(it++) != ',');
            CascadingStyleParser::ignoreWhitespace(it, end);

            _fence_value_return(nullptr, !VectorParser::readFloat(it, end, val));
            by[2][1] = val;
            _fence_value_return(nullptr, it == end || *(it++) != ')');
            CascadingStyleParser::ignoreWhitespace(it, end);

            ret = by * ret;
        }
    }
    return ret;
}

_push_nowarn_msvc(_clWarn_msvc_overflow);
bool VectorParser::parsePath(std::string_view attrVal, std::vector<PathCommand>& out)
{
    _fence_value_return(false, attrVal.empty());

    constexpr auto isCommand = [](char c) -> bool
    {
        return c == 'M' || c == 'm' || c == 'L' || c == 'l' || c == 'H' || c == 'h' || c == 'V' || c == 'v' || c == 'C' || c == 'c' || c == 'S' || c == 's' || c == 'Q' ||
            c == 'q' || c == 'T' || c == 't' || c == 'A' || c == 'a' || c == 'Z' || c == 'z';
    };

    const char* it = std::to_address(attrVal.begin());
    const char* const end = std::to_address(attrVal.end());

    CascadingStyleParser::ignoreWhitespace(it, end); // Start ourselves at the first real character.

    auto readNextCommand = [&]() -> char
    {
        _fence_value_return(false, it == end);

        if (isCommand(*it))
        {
            char ret = *it;
            ++it;
            CascadingStyleParser::ignoreWhitespace(it, end);
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

        _fence_value_return(false, res.ec != std::errc() || it == res.ptr || ret == HUGE_VALF || std::isinf(ret) || std::isnan(ret));
        it = res.ptr;
        out += ret;

        CascadingStyleParser::ignoreWhitespace(it, end);

        return true;
    };
    auto readDelimiter = [&]() -> bool
    {
        _fence_value_return(false, it == end);
        _fence_value_return(false, *it != ',' && *it != '-');

        if (*it == ',')
            ++it;
        CascadingStyleParser::ignoreWhitespace(it, end);
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
    glm::vec2 cur(0.0f);

    char command; // Always reinitialized in loop.
    char injectedNextCommand = 0;
    auto readNextSetInjectedCommandIfNeeded = [&](char injectAs) -> void
    {
        readDelimiter();
        if (it != end && !isCommand(*it))
            injectedNextCommand = injectAs;
    };

    glm::vec2 to(0.0f, 0.0f);
    while (it != end)
    {
        glm::vec2 c1, c2;
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
            to = glm::vec2(0.0f);
            goto AnyMoveTo;
        case 'm':
            to = cur;
        AnyMoveTo:
            pathBegin = ssz(out.size());
            _fence_value_return(false, !readAddNextFloatPair(to.x, to.y));
            out.emplace_back(to);
            readNextSetInjectedCommandIfNeeded(command == 'M' ? 'L' : 'l');
            break;

        case 'L':
            to = glm::vec2(0.0f);
            goto AnyLineTo;
        case 'l':
            to = cur;
        AnyLineTo:
            _fence_value_return(false, !readAddNextFloatPair(to.x, to.y));
            out.emplace_back(cur, to);
            readNextSetInjectedCommandIfNeeded(command);
            break;

        case 'H':
            to = glm::vec2(0.0f, cur.y);
            horizReadTo = &to.x;
            goto AnyHorizTo;
        case 'h':
            to = cur;
            horizReadTo = &to.x;
            goto AnyHorizTo;
        case 'V':
            to = glm::vec2(cur.x, 0.0f);
            horizReadTo = &to.y;
            goto AnyHorizTo;
        case 'v':
            to = cur;
            horizReadTo = &to.y;
        AnyHorizTo:
            _fence_value_return(false, !readAddNextFloat(*horizReadTo));
            out.emplace_back(cur, to);
            readNextSetInjectedCommandIfNeeded(command);
            break;

        case 'C':
            c1 = glm::vec2(0.0f);
            [[fallthrough]];
        case 'S':
            c2 = glm::vec2(0.0f);
            to = glm::vec2(0.0f);
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
                        c1 = cur - (out.back().dc.ctrl2 - cur);
                    else
                        c1 = cur;
                }
                else
                {
                    _fence_value_return(false, !readAddNextFloatPair(c1.x, c1.y));
                    readDelimiter();
                }

                _fence_value_return(false, !readAddNextFloatPair(c2.x, c2.y));
                readDelimiter();
                _fence_value_return(false, !readAddNextFloatPair(to.x, to.y));

                out.emplace_back(cur, c1, c2, to);

                readNextSetInjectedCommandIfNeeded(command);
            }
            break;

        case 'Q':
            c1 = glm::vec2(0.0f);
            [[fallthrough]];
        case 'T':
            to = glm::vec2(0.0f);
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
                        c1 = cur - (out.back().dc.ctrl1 - cur);
                    else
                        c1 = cur;
                }
                else
                {
                    _fence_value_return(false, !readAddNextFloatPair(c1.x, c1.y));
                    readDelimiter();
                }

                _fence_value_return(false, !readAddNextFloatPair(to.x, to.y));
                out.emplace_back(cur, c1, to);
                readNextSetInjectedCommandIfNeeded(command);
            }
            break;

        case 'A':
        case 'a':
            return false; // Not even going to try.

        case 'Z':
        case 'z':
            out.emplace_back(pathBegin, ssz(out.size()));
            pathBegin = ssz(out.size());
            break;
        }

        cur = to;
    }

    return true;
}
_pop_nowarn_msvc();
