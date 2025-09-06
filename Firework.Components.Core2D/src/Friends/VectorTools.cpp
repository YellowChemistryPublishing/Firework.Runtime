#include "VectorTools.h"

#include <charconv>

#include <Friends/ShapeRenderer.h>

using namespace Firework;

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
bool VectorTools::shapeProcessCurvesFromOutline(const std::span<const ShapeOutlinePoint> points, std::vector<struct ShapePoint>& outPoints, std::vector<uint16_t>& outInds,
                                                std::vector<struct ShapePoint>& outTriPoints, std::vector<uint16_t>& outTriInds)
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

            outTriPoints.emplace_back(ShapePoint { .x = converted.p1.x, .y = converted.p1.y, .xCtrl = 0.0f, .yCtrl = 1.0f });
            outTriPoints.emplace_back(ShapePoint { .x = converted.p2.x, .y = converted.p2.y, .xCtrl = 0.0f, .yCtrl = 1.0f });
            outTriPoints.emplace_back(ShapePoint { .x = converted.p3.x, .y = converted.p3.y, .xCtrl = 0.0f, .yCtrl = 1.0f });

            u16 i = outPoints.size();
            for (const u16 sub : { 5_u16, 4_u16, 3_u16, 3_u16, 2_u16, 1_u16 }) outInds.emplace_back(+(i - sub));

            i = outTriPoints.size();
            for (const u16 sub : { 3_u16, 2_u16, 1_u16 }) outTriInds.emplace_back(+(i - sub));
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
