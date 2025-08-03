#pragma once

#include "Firework.Components.Core2D.Exports.h"

_push_nowarn_conv_comp();
#include <array>
#include <glm/mat4x4.hpp>
_pop_nowarn_conv_comp();

#include <Friends/Color.h>
#include <GL/Geometry.h>

namespace
{
    struct ComponentStaticInit;
}

namespace Firework::GL
{
    class GeometryProgram;
}

_push_nowarn_msvc(_clWarn_msvc_export_interface);
namespace Firework
{
    struct alignas(float) FringePoint
    {
        float x, y, z = 1.0f; // Leave `z` as 1.0f unless you're really confident in what you're doing.

        friend constexpr bool operator==(const FringePoint&, const FringePoint&) = default;
    };

    class _fw_cc2d_api FringeRenderer final
    {
        static GL::GeometryProgram drawProgram;

        [[nodiscard]] static bool renderInitialize();

        GL::StaticMesh fill = nullptr;
    public:
        FringeRenderer(std::nullptr_t)
        { }
        FringeRenderer(std::span<const FringePoint> points, std::span<const ssz> closedPathRanges);
        FringeRenderer(std::span<const FringePoint> points) : FringeRenderer(points, std::array<ssz, 2> { 0_z, ssz(points.size()) })
        { }
        FringeRenderer(const FringeRenderer&) = delete;
        FringeRenderer(FringeRenderer&& other)
        {
            swap(*this, other);
        }

        operator bool()
        {
            return this->fill;
        }

        FringeRenderer& operator=(const FringeRenderer&) = delete;
        FringeRenderer& operator=(FringeRenderer&& other)
        {
            swap(*this, other);
            return *this;
        }

        [[nodiscard]] bool submitDraw(float renderIndex, glm::mat4 transform, Color color = Color::unknown) const;

        friend void swap(FringeRenderer& a, FringeRenderer& b)
        {
            using std::swap;

            swap(a.fill, b.fill);
        }

        friend struct ::ComponentStaticInit;
    };
} // namespace Firework
_pop_nowarn_msvc();
