#pragma once

#include "Firework.Components.Core2D.Exports.h"

_push_nowarn_conv_comp();
#include <array>
#include <glm/mat4x4.hpp>
#include <module/sys>
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
    class FringeRenderer;

    enum class FillRule : uint_least8_t
    {
        EvenOdd,
        NonZero
    };

    struct alignas(float) ShapePoint
    {
        float x, y, z = 1.0f; // Leave `z` as 1.0f unless you're really confident in what you're doing.
        float xCtrl = 0.0f, yCtrl = 0.0f;

        constexpr friend bool operator==(const ShapePoint&, const ShapePoint&) = default;
    };

    class _fw_cc2d_api ShapeRenderer final
    {
        static GL::GeometryProgram drawProgram;

        static GL::StaticMesh unitSquare;

        [[nodiscard]] static bool renderInitialize();

        GL::StaticMesh fill = nullptr;
        u32 curvePointsSize = 0_u32;
        u32 curveIndsBegin = 0_u32;
        u32 curveIndsEnd = 0_u32;
    public:
        ShapeRenderer(std::nullptr_t)
        { }
        ShapeRenderer(std::span<const ShapePoint> points, std::span<const uint16_t> inds, u32 curveIndsBegin);
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

        [[nodiscard]] bool submitDrawStencil(float renderIndex, glm::mat4 shape, FillRule fillRule = FillRule::EvenOdd) const;
        _push_nowarn_gcc(_clWarn_gcc_c_cast);
        _push_nowarn_clang(_clWarn_clang_c_cast);
        [[nodiscard]] static bool submitDrawCover(float renderIndex, glm::mat4 clip, u8 refZero = 0_u8, Color color = Color::unknown,
                                                  u64 blendState = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_ALWAYS |
                                                      BGFX_STATE_BLEND_ALPHA,
                                                  u32 stencilTest = BGFX_STENCIL_TEST_NOTEQUAL | BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP);
        _pop_nowarn_clang();
        _pop_nowarn_gcc();

        friend void swap(ShapeRenderer& a, ShapeRenderer& b)
        {
            using std::swap;

            swap(a.curvePointsSize, b.curvePointsSize);
            swap(a.curveIndsBegin, b.curveIndsBegin);
            swap(a.curveIndsEnd, b.curveIndsEnd);
            swap(a.fill, b.fill);
        }

        friend struct ::ComponentStaticInit;
        friend class Firework::FringeRenderer;
    };
} // namespace Firework
_pop_nowarn_msvc();
