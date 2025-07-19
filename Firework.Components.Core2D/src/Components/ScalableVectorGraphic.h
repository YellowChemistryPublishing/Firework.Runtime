#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <mutex>
#include <robin_hood.h>

#include <Friends/FilledPathRenderer.h>
#include <Friends/VectorTools.h>
#include <GL/Transform.h>
#include <Library/Property.h>

namespace
{
    struct ComponentStaticInit;
}

namespace Firework::PackageSystem
{
    class ExtensibleMarkupPackageFile;
}

namespace Firework
{
    class Entity;
    class RectTransform;

    class _fw_cc2d_api [[fw::component]] ScalableVectorGraphic final
    {
        enum class RenderableType
        {
            FilledPath,
            NoOp
        };
        struct FilledPathRenderable
        {
            FilledPathRenderer rend;
            Color col;

            FilledPathRenderable(FilledPathRenderer rend, Color col) : rend(std::move(rend)), col(col)
            { }
            FilledPathRenderable(FilledPathRenderable&& other)
            {
                swap(*this, other);
            }

            friend void swap(FilledPathRenderable& a, FilledPathRenderable& b)
            {
                using std::swap;

                swap(a.rend, b.rend);
                swap(a.col, b.col);
            }
        };
        struct Renderable
        {
            union
            {
                FilledPathRenderable filledPath;
            };
            const RenderableType type = RenderableType::NoOp;

            Renderable() = default;
            Renderable(FilledPathRenderable filledPath) : filledPath(std::move(filledPath)), type(RenderableType::FilledPath)
            { }
            Renderable(Renderable&& other)
            {
                swap(*this, other);
            }
            ~Renderable()
            {
                switch (this->type)
                {
                case RenderableType::FilledPath:
                    this->filledPath.~FilledPathRenderable();
                    break;
                }
            }

            friend void swap(Renderable& a, Renderable& b)
            {
                if (&a != &b)
                    std::swap_ranges(_asr(byte*, &a), _asr(byte*, &a) + sizeof(Renderable), _asr(byte*, &b));
            }
        };

        static robin_hood::unordered_map<PackageSystem::ExtensibleMarkupPackageFile*, std::shared_ptr<std::vector<Renderable>>> loadedSvgs;

        std::shared_ptr<RectTransform> rectTransform = nullptr;

        std::shared_ptr<PackageSystem::ExtensibleMarkupPackageFile> _svgFile = nullptr;

        bool dirty = false;
        PackageSystem::ExtensibleMarkupPackageFile* deferOldSvg = nullptr;

        struct RenderData
        {
            std::shared_ptr<std::vector<Renderable>> toRender;

            VectorTools::Viewbox vb;
            GL::RenderTransform tf;

            std::mutex toRenderLock;
        };
        std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();

        void onAttach(Entity& entity);

        std::shared_ptr<std::vector<Renderable>> findOrCreateRenderablePath(PackageSystem::ExtensibleMarkupPackageFile& svg);
        //                           v Never null, but also may be invalid, so passed by ptr, not ref.
        void buryLoadedSvgIfOrphaned(PackageSystem::ExtensibleMarkupPackageFile* svg);

        void renderOffload(ssz renderIndex);
    public:
        Property<std::shared_ptr<PackageSystem::ExtensibleMarkupPackageFile>, std::shared_ptr<PackageSystem::ExtensibleMarkupPackageFile>> svgFile {
            [this]() -> std::shared_ptr<PackageSystem::ExtensibleMarkupPackageFile> { return this->_svgFile; },
            [this](std::shared_ptr<PackageSystem::ExtensibleMarkupPackageFile> value) -> void
        {
            _fence_value_return(void(), this->_svgFile == value);

            this->dirty = true;
            this->_svgFile = std::move(value);
        }
        };

        friend struct ::ComponentStaticInit;
        friend class Firework::Entity;
    };
} // namespace Firework