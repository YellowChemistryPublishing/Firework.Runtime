#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <module/sys>

#include <GL/Geometry.h>
#include <GL/Shader.h>
#include <GL/Transform.h>

namespace Firework
{
    namespace Internal
    {
        struct ComponentCore2DStaticInit;
    }

    struct _packed FilledPathPoint
    {
        float x, y, z = 1.0f; // Leave `z` as 1.0f unless you're really confident in what you're doing.
    };

    class __firework_componentcore2d_api FilledPathRenderer final
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

        [[nodiscard]] bool submitDrawStencil(sz renderIndex, GL::RenderTransform shape, bool forceHole = false);
        [[nodiscard]] static bool submitDraw(sz renderIndex, GL::RenderTransform clip, u8 whenStencil = ~0_u8);

        friend inline void swap(FilledPathRenderer& a, FilledPathRenderer& b)
        {
            using std::swap;

            swap(a.fill, b.fill);
        }

        friend struct Firework::Internal::ComponentCore2DStaticInit;
    };
} // namespace Firework
