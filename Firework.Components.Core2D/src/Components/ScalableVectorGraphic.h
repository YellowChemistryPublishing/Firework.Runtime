#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <mutex>
#include <robin_hood.h>

#include <Friends/ShapeRenderer.h>
#include <Friends/VectorTools.h>
#include <Library/Property.h>

namespace
{
    struct ComponentStaticInit;
}

namespace Firework::PackageSystem
{
    class ExtensibleMarkupPackageFile;
}

_push_nowarn_msvc(_clWarn_msvc_export_interface);
namespace Firework
{
    class Entity;
    class RectTransform;

    class _fw_cc2d_api ScalableVectorGraphic final
    {
        enum class RenderableType
        {
            FilledPath,
            NoOp
        };
        struct FilledPathRenderable
        {
            ShapeRenderer rend = nullptr;
            glm::mat4 tf = glm::mat4(1.0f);
            Color col = Color::unknown;

            FilledPathRenderable(ShapeRenderer rend, const glm::mat4 tf, Color col = Color::unknown) : rend(std::move(rend)), tf(tf), col(col)
            { }
            FilledPathRenderable(FilledPathRenderable&& other)
            {
                swap(*this, other);
            }

            friend void swap(FilledPathRenderable& a, FilledPathRenderable& b)
            {
                using std::swap;

                swap(a.rend, b.rend);
                swap(a.tf, b.tf);
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

            Renderable() { };
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
                case RenderableType::NoOp:
                default:;
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
            glm::mat4 tf;

            std::mutex lock;
        };
        std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();

        void onAttach(Entity& entity);

        std::shared_ptr<std::vector<Renderable>> findOrCreateRenderablePath(PackageSystem::ExtensibleMarkupPackageFile& svg);
        //                           v Never null, but also may be invalid, so passed by ptr, not ref.
        void buryLoadedSvgIfOrphaned(PackageSystem::ExtensibleMarkupPackageFile* svg);

        void lateRenderOffload(ssz renderIndex);
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
_pop_nowarn_msvc();
