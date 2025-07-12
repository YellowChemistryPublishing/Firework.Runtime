#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <module/sys>

#include <GL/Geometry.h>

namespace
{
    struct ComponentStaticInit;
}

namespace Firework
{
    namespace GL
    {
        struct RenderTransform;

        class GeometryProgramHandle;
    }

    struct _packed FilledPathPoint
    {
        float x, y, z = 1.0f; // Leave `z` as 1.0f unless you're really confident in what you're doing.
        float xCtrl = 0.0f, yCtrl = 0.0f;
    };

    class _fw_cc2d_api FilledPathRenderer final
    {
        static GL::GeometryProgramHandle program;
        static GL::StaticMeshHandle unitSquare;

        [[nodiscard]] static bool renderInitialize();

        GL::StaticMeshHandle fill = nullptr;
    public:
        inline FilledPathRenderer(std::nullptr_t = nullptr)
        { }
        FilledPathRenderer(std::span<FilledPathPoint> closedPath);
        inline FilledPathRenderer(const FilledPathRenderer&) = delete;
        inline FilledPathRenderer(FilledPathRenderer&& other)
        {
            swap(*this, other);
        }
        inline ~FilledPathRenderer()
        {
            if (this->fill) [[likely]]
                this->fill.destroy();
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

        [[nodiscard]] bool submitDrawStencil(ssz renderIndex, GL::RenderTransform shape, bool forceHole = false);
        [[nodiscard]] static bool submitDraw(ssz renderIndex, GL::RenderTransform clip, u8 whenStencil = ~0_u8);

        friend inline void swap(FilledPathRenderer& a, FilledPathRenderer& b)
        {
            using std::swap;

            swap(a.fill, b.fill);
        }

        friend struct ::ComponentStaticInit;
    };
} // namespace Firework
