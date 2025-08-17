#include "VectorTools.h"

#include <charconv>

#include <Friends/ShapeRenderer.h>

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

_push_nowarn_msvc(_clWarn_msvc_overflow);
bool VectorTools::parsePath(std::string_view attrVal, std::vector<PathCommand>& out)
{
    _fence_value_return(false, attrVal.empty());

    constexpr auto isCommand = [](char c) -> bool
    {
        return c == 'M' || c == 'm' || c == 'L' || c == 'l' || c == 'H' || c == 'h' || c == 'V' || c == 'v' || c == 'C' || c == 'c' || c == 'S' || c == 's' || c == 'Q' ||
            c == 'q' || c == 'T' || c == 't' || c == 'A' || c == 'a' || c == 'Z' || c == 'z';
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

        _fence_value_return(false, res.ec != std::errc() || it == res.ptr || ret == HUGE_VALF || std::isinf(ret) || std::isnan(ret));
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
    glm::vec2 cur(0.0f);

    char command; // Always reinitialized in loop.
    char injectedNextCommand = 0;
    auto readNextSetInjectedCommandIfNeeded = [&](char injectAs) -> void
    {
        if (readDelimiter())
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
                    _fence_value_return(false, !readDelimiter());
                }

                _fence_value_return(false, !readAddNextFloatPair(c2.x, c2.y));
                _fence_value_return(false, !readDelimiter());
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
                    _fence_value_return(false, !readDelimiter());
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

// https://www.cemyuksel.com/research/papers/quadratic_approximation_of_cubic_curves.pdf
VectorTools::QuadApproxCubic VectorTools::cubicBeizerToQuadratic(glm::vec2 p1, glm::vec2 c1, glm::vec2 c2, glm::vec2 p2)
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

float VectorTools::fastQuadraticBezierLength(glm::vec2 p1, glm::vec2 c, glm::vec2 p2)
{
    return (glm::length(p2 - p1) + glm::length(c - p1) + glm::length(p2 - c)) * 0.5f;
}
bool VectorTools::quadraticBezierToLines(const glm::vec2 p1, const glm::vec2 c, const glm::vec2 p2, const ssz segments, std::vector<glm::vec2>& out)
{
    _fence_value_return(false, segments < 1_z);

    for (ssz i = 0; i < segments + 1_z; i++)
    {
        const float t = float(+i) / float(+segments);
        out.emplace_back((1.0f - t) * (1.0f - t) * p1 + 2.0f * t * (1.0f - t) * c + t * t * p2);
    }
    return true;
}
bool VectorTools::quadraticBezierToLines(const glm::vec2 p1, const glm::vec2 c, const glm::vec2 p2, const float segmentLength, std::vector<glm::vec2>& out)
{
    const float len = VectorTools::fastQuadraticBezierLength(p1, c, p2);
    const float segmentsUnrounded = std::ceilf(len / segmentLength);
    _fence_value_return(false, segmentsUnrounded >= float(std::numeric_limits<ssz::underlying_type>::max()));

    ssz segments = segmentsUnrounded;
    return VectorTools::quadraticBezierToLines(p1, c, p2, segments, out);
}

bool VectorTools::shapeTrianglesFromOutline(std::span<const ShapeOutlinePoint> points, std::vector<struct ShapePoint>& outPoints, std::vector<uint16_t>& outInds,
                                            const glm::vec2 windAround)
{
    _fence_value_return(false, points.size() < 3);

    const u16 outPointsBeg = outPoints.size();
    outPoints.emplace_back(ShapePoint { .x = windAround.x, .y = windAround.y, .xCtrl = 0.0f, .yCtrl = 1.0f });
    for (const auto& pt : points)
        if (!pt.isCtrl)
            outPoints.emplace_back(ShapePoint { .x = pt.x, .y = pt.y, .xCtrl = 0.0f, .yCtrl = 1.0f });
    const u16 outPointsEnd = outPoints.size();

    for (u16 i = outPointsBeg + 1_u16; i < outPointsEnd - 1_u16; i++)
    {
        outInds.emplace_back(+outPointsBeg);
        outInds.emplace_back(+i);
        outInds.emplace_back(+(i + 1_u16));
    }
    outInds.emplace_back(+outPointsBeg);
    outInds.emplace_back(+(outPointsEnd - 1_u16));
    outInds.emplace_back(+(outPointsBeg + 1_u16));

    return true;
}
bool VectorTools::shapeProcessCurvesFromOutline(const std::span<const ShapeOutlinePoint> points, std::vector<struct ShapePoint>& outPoints, std::vector<uint16_t>& outInds)
{
    _fence_value_return(false, points.size() < 3);

    for (auto ptIt = points.cbegin(); ptIt < --(--points.cend()); ++ptIt)
    {
        const auto ptIt2 = ++decltype(ptIt)(ptIt);
        const auto ptIt3 = ++std::remove_const_t<decltype(ptIt2)>(ptIt2);
        if (!ptIt2->isCtrl || ptIt->isCtrl /* Probably an error case, but let's be extra fault-tolerant. (1) */)
            continue; // Not a control point, so no curve to fill.

        if (ptIt3->isCtrl) // Cubic bezier.
        {
            const auto ptIt4 = ++std::remove_const_t<decltype(ptIt3)>(ptIt3);
            _fence_value_return(false, ptIt4 == points.cend());
            if (ptIt4->isCtrl)
                continue; // See (1.)

            VectorTools::QuadApproxCubic converted =
                VectorTools::cubicBeizerToQuadratic(glm::vec2(ptIt->x, ptIt->y), glm::vec2(ptIt2->x, ptIt2->y), glm::vec2(ptIt3->x, ptIt3->y), glm::vec2(ptIt4->x, ptIt4->y));

            outPoints.emplace_back(ShapePoint { .x = converted.p1.x, .y = converted.p1.y, .xCtrl = -1.0f, .yCtrl = 1.0f });
            outPoints.emplace_back(ShapePoint { .x = converted.c1.x, .y = converted.c1.y, .xCtrl = 0.0f, .yCtrl = -1.0f });
            outPoints.emplace_back(ShapePoint { .x = converted.p2.x, .y = converted.p2.y, .xCtrl = 1.0f, .yCtrl = 1.0f });
            outPoints.emplace_back(ShapePoint { .x = converted.c2.x, .y = converted.c2.y, .xCtrl = 0.0f, .yCtrl = -1.0f });
            outPoints.emplace_back(ShapePoint { .x = converted.p3.x, .y = converted.p3.y, .xCtrl = -1.0f, .yCtrl = 1.0f });

            outPoints.emplace_back(ShapePoint { .x = converted.p1.x, .y = converted.p1.y, .xCtrl = 0.0f, .yCtrl = 1.0f });
            outPoints.emplace_back(ShapePoint { .x = converted.p2.x, .y = converted.p2.y, .xCtrl = 0.0f, .yCtrl = 1.0f });
            outPoints.emplace_back(ShapePoint { .x = converted.p3.x, .y = converted.p3.y, .xCtrl = 0.0f, .yCtrl = 1.0f });

            u16 i = outPoints.size();
            for (const u16 sub : { 8_u16, 7_u16, 6_u16, 6_u16, 5_u16, 4_u16, 3_u16, 2_u16, 1_u16 }) outInds.emplace_back(+(i - sub));
        }
        else // Quadratic bezier.
        {
            if (ptIt3->isCtrl)
                continue; // See (1.)

            outPoints.emplace_back(ShapePoint { .x = ptIt->x, .y = ptIt->y, .xCtrl = -1.0f, .yCtrl = 1.0f });
            outPoints.emplace_back(ShapePoint { .x = ptIt2->x, .y = ptIt2->y, .xCtrl = 0.0f, .yCtrl = -1.0f });
            outPoints.emplace_back(ShapePoint { .x = ptIt3->x, .y = ptIt3->y, .xCtrl = 1.0f, .yCtrl = 1.0f });

            u16 i = outPoints.size();
            for (const u16 sub : { 3_u16, 2_u16, 1_u16 }) outInds.emplace_back(+(i - sub));
        }

        ptIt = ptIt3; // Go to end of this curve.
    }

    return true;
}
