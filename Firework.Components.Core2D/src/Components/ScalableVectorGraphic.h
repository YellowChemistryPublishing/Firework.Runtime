#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <mutex>
#include <robin_hood.h>

#include <Friends/FilledPathRenderer.h>
#include <GL/Transform.h>
#include <Library/Property.h>

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
        static robin_hood::unordered_map<PackageSystem::ExtensibleMarkupPackageFile*, std::shared_ptr<std::vector<FilledPathRenderer>>> loadedSvgs;

        std::shared_ptr<RectTransform> rectTransform = nullptr;

        std::shared_ptr<PackageSystem::ExtensibleMarkupPackageFile> _svgFile = nullptr;

        bool dirty = false;
        PackageSystem::ExtensibleMarkupPackageFile* deferOldSvg = nullptr;

        struct RenderData
        {
            std::shared_ptr<std::vector<FilledPathRenderer>> toRender;
            GL::RenderTransform tf;
            std::mutex toRenderLock;
        };
        std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();

        void onAttach(Entity& entity);

        std::shared_ptr<std::vector<FilledPathRenderer>> findOrCreateRenderablePath(PackageSystem::ExtensibleMarkupPackageFile* svg);
        void buryLoadedSvgIfOrphaned(PackageSystem::ExtensibleMarkupPackageFile* svg);

        void renderOffload(ssz renderIndex);
    public:
        Property<std::shared_ptr<PackageSystem::ExtensibleMarkupPackageFile>, std::shared_ptr<PackageSystem::ExtensibleMarkupPackageFile>> svgFile {
            [this]() -> std::shared_ptr<PackageSystem::ExtensibleMarkupPackageFile> { return this->_svgFile; },
            [this](std::shared_ptr<PackageSystem::ExtensibleMarkupPackageFile> value) -> void
        {
            this->dirty = true;
            this->_svgFile = std::move(value);
        }
        };
    };
} // namespace Firework