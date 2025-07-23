#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <array>
#include <glm/mat4x4.hpp>
#include <module/sys>

#include <Friends/Color.h>
#include <GL/Geometry.h>

namespace
{
    struct ComponentStaticInit;
}

namespace Firework::GL
{
    class GeometryProgram;
} // namespace Firework::GL

namespace Firework
{
    struct _packed FilledPathPoint
    {
        float x, y, z = 1.0f; // Leave `z` as 1.0f unless you're really confident in what you're doing.
        float xCtrl = 0.0f, yCtrl = 0.0f;
    };

    class _fw_cc2d_api FilledPathRenderer final
    {
        static GL::GeometryProgram stencilProgram;
        static GL::GeometryProgram drawProgram;

        static GL::StaticMesh unitSquare;

        [[nodiscard]] static bool renderInitialize();

        GL::StaticMesh fill = nullptr;
    public:
        inline FilledPathRenderer(std::nullptr_t)
        { }
        FilledPathRenderer(std::span<const FilledPathPoint> points, std::span<const ssz> closedPathRanges);
        FilledPathRenderer(std::span<const FilledPathPoint> points) : FilledPathRenderer(points, std::array<ssz, 2> { 0_z, ssz(points.size()) })
        { }
        inline FilledPathRenderer(const FilledPathRenderer&) = delete;
        inline FilledPathRenderer(FilledPathRenderer&& other)
        {
            swap(*this, other);
        }

        inline operator bool()
        {
            return this->fill;
        }

        inline FilledPathRenderer& operator=(const FilledPathRenderer&) = delete;
        inline FilledPathRenderer& operator=(FilledPathRenderer&& other)
        {
            swap(*this, other);
            return *this;
        }

        [[nodiscard]] bool submitDrawStencil(ssz renderIndex, glm::mat4 shape, bool forceHole = false) const;
        [[nodiscard]] static bool submitDraw(ssz renderIndex, glm::mat4 clip, u8 whenStencil = ~0_u8, Color color = Color::unknown);

        friend inline void swap(FilledPathRenderer& a, FilledPathRenderer& b)
        {
            using std::swap;

            swap(a.fill, b.fill);
        }

        friend struct ::ComponentStaticInit;
    };
} // namespace Firework
