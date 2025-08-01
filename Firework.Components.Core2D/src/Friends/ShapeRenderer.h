#pragma once

#include "Firework.Components.Core2D.Exports.h"

_push_nowarn_gcc(_clWarn_gcc_sign_compare);
_push_nowarn_gcc(_clWarn_gcc_sign_conversion);
_push_nowarn_clang(_clWarn_clang_sign_compare);
_push_nowarn_clang(_clWarn_clang_sign_conversion);
#include <array>
#include <glm/mat4x4.hpp>
#include <module/sys>
_pop_nowarn_clang();
_pop_nowarn_clang();
_pop_nowarn_gcc();
_pop_nowarn_gcc();

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
    struct alignas(float) ShapePoint
    {
        float x, y, z = 1.0f; // Leave `z` as 1.0f unless you're really confident in what you're doing.
        float xCtrl = 0.0f, yCtrl = 0.0f;
    };

    class _fw_cc2d_api ShapeRenderer final
    {
        static GL::GeometryProgram stencilProgram;
        static GL::GeometryProgram drawProgram;

        static GL::StaticMesh unitSquare;

        [[nodiscard]] static bool renderInitialize();

        GL::StaticMesh fill = nullptr;
    public:
        ShapeRenderer(std::nullptr_t)
        { }
        ShapeRenderer(std::span<const ShapePoint> points, std::span<const ssz> closedPathRanges);
        ShapeRenderer(std::span<const ShapePoint> points) : ShapeRenderer(points, std::array<ssz, 2> { 0_z, ssz(points.size()) })
        { }
        ShapeRenderer(const ShapeRenderer&) = delete;
        ShapeRenderer(ShapeRenderer&& other)
        {
            swap(*this, other);
        }

        operator bool()
        {
            return this->fill;
        }

        ShapeRenderer& operator=(const ShapeRenderer&) = delete;
        ShapeRenderer& operator=(ShapeRenderer&& other)
        {
            swap(*this, other);
            return *this;
        }

        [[nodiscard]] bool submitDrawStencil(float renderIndex, glm::mat4 shape, bool forceHole = false) const;
        [[nodiscard]] static bool submitDraw(float renderIndex, glm::mat4 clip, u8 whenStencil = ~0_u8, Color color = Color::unknown);

        friend void swap(ShapeRenderer& a, ShapeRenderer& b)
        {
            using std::swap;

            swap(a.fill, b.fill);
        }

        friend struct ::ComponentStaticInit;
    };
} // namespace Firework
_pop_nowarn_msvc();
