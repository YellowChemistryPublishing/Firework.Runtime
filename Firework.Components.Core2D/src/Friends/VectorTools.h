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
    public:
        VectorTools() = delete;

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
