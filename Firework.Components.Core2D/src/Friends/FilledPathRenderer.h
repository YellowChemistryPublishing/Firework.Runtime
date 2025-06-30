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
        float x, y, z = 1.0f; // Leave as 1.0f unless you're really confident in what you're doing.
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
        template <typename Container>
        requires sys::IEnumerable<Container, FilledPathPoint> && sys::ISizeable<Container>
        inline FilledPathRenderer(Container&& closedPath)
        {
            std::vector<FilledPathPoint> verts;
            if constexpr (requires { sz(std::size(closedPath)); })
                verts.reserve(+sz(std::size(closedPath)));
            verts.insert(verts.begin(), std::begin(closedPath), std::end(closedPath));

            std::vector<uint16_t> inds;
            if constexpr (requires { sz(std::size(closedPath)); })
                inds.reserve(+((sz(std::size(closedPath)) - 2_uz) * 3_uz));

            for (u16 i = 1; i < u16(std::size(closedPath)) - 1_u16; i++)
            {
                inds.push_back(0);
                inds.push_back(+(i + 1_u16));
                inds.push_back(+i);
            }

            this->fill = GL::StaticMeshHandle::create(
                std::data(verts), std::size(verts) * sizeof(decltype(verts)::value_type),
                GL::VertexLayout::create({ GL::VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 } }), std::data(inds),
                std::size(inds) * sizeof(decltype(inds)::value_type));
        }
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
        [[nodiscard]] static bool submitDraw(sz renderIndex, GL::RenderTransform clip, u8 whenStencil = ~u8(0));

        friend inline void swap(FilledPathRenderer& a, FilledPathRenderer& b)
        {
            using std::swap;

            swap(a.fill, b.fill);
        }

        friend struct Firework::Internal::ComponentCore2DStaticInit;
    };
} // namespace Firework
