#include "ScalableVectorGraphic.h"

#include <Components/RectTransform.h>
#include <Core/CoreEngine.h>
#include <Core/Debug.h>
#include <EntityComponentSystem/Entity.h>
#include <Friends/VectorTools.h>
#include <PackageSystem/ExtensibleMarkupFile.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::PackageSystem;

robin_hood::unordered_map<PackageSystem::ExtensibleMarkupPackageFile*, std::shared_ptr<std::vector<FilledPathRenderer>>> ScalableVectorGraphic::loadedSvgs;

void ScalableVectorGraphic::onAttach(Entity& entity)
{
    this->rectTransform = entity.getOrAddComponent<RectTransform>();
}

std::shared_ptr<std::vector<FilledPathRenderer>> ScalableVectorGraphic::findOrCreateRenderablePath(PackageSystem::ExtensibleMarkupPackageFile& svg)
{
    const auto svgIt = ScalableVectorGraphic::loadedSvgs.find(&svg);
    _fence_value_return(svgIt->second, svgIt != ScalableVectorGraphic::loadedSvgs.end());

    std::shared_ptr<std::vector<FilledPathRenderer>> ret = std::make_shared<std::vector<FilledPathRenderer>>();

    const auto traverse = [&](const auto& traverse, const pugi::xml_node& node) -> void
    {
        std::basic_string_view<pugi::char_t> tag = node.name();
        if (tag == PUGIXML_TEXT("svg"))
        {
            for (const pugi::xml_node& child : node) traverse(traverse, child);
        }
        else if (tag == PUGIXML_TEXT("g"))
        {
            for (const pugi::xml_node& child : node) traverse(traverse, child);
        }
        else if (tag == PUGIXML_TEXT("path"))
        {
            std::vector<VectorTools::PathCommand> pathCommands;
            _fence_value_return(void(), !VectorTools::parsePath(node.attribute("d").value(), pathCommands));

            std::vector<FilledPathPoint> paths;
            std::vector<ssz> spans { 0_z };
            float alternatePtCtrl = 1.0f;

            for (const VectorTools::PathCommand& pc : pathCommands)
            {
                switch (pc.type)
                {
                case VectorTools::PathCommandType::MoveTo:
                    paths.emplace_back(FilledPathPoint { .x = pc.moveTo.to.x, .y = pc.moveTo.to.y, .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });
                    break;
                case VectorTools::PathCommandType::LineTo:
                    paths.emplace_back(FilledPathPoint { .x = pc.lineTo.to.x, .y = pc.lineTo.to.y, .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });
                    break;
                case VectorTools::PathCommandType::CubicTo:
                    {
                        VectorTools::QuadApproxCubic converted = VectorTools::cubicBeizerToQuadratic(pc.cubicTo.from, pc.cubicTo.ctrl1, pc.cubicTo.ctrl2, pc.cubicTo.to);
                        paths.emplace_back(FilledPathPoint { .x = converted.c1.x, .y = converted.c1.y, .xCtrl = 0.0f, .yCtrl = -1.0f });
                        paths.emplace_back(FilledPathPoint { .x = converted.p2.x, .y = converted.p2.y, .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });
                        paths.emplace_back(FilledPathPoint { .x = converted.c2.x, .y = converted.c2.y, .xCtrl = 0.0f, .yCtrl = -1.0f });
                        paths.emplace_back(FilledPathPoint { .x = converted.p3.x, .y = converted.p3.y, .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });
                    }
                    break;
                case VectorTools::PathCommandType::QuadraticTo:
                    paths.emplace_back(FilledPathPoint { .x = pc.quadraticTo.ctrl.x, .y = pc.quadraticTo.ctrl.y, .xCtrl = 0.0f, .yCtrl = -1.0f });
                    paths.emplace_back(FilledPathPoint { .x = pc.quadraticTo.to.x, .y = pc.quadraticTo.to.y, .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });
                    break;
                case VectorTools::PathCommandType::ClosePath:
                    spans.emplace_back(ssz(paths.size()));
                    break;
                }
            }

            if (FilledPathRenderer pr = FilledPathRenderer(paths, spans))
                ret->emplace_back(std::move(pr));
        }
        // Otherwise, do nothing.
    };
    for (const pugi::xml_node& node : *this->_svgFile) traverse(traverse, node);

    return ret;
}
void ScalableVectorGraphic::buryLoadedSvgIfOrphaned(PackageSystem::ExtensibleMarkupPackageFile* svg)
{
    auto svgIt = ScalableVectorGraphic::loadedSvgs.find(svg);
    _fence_value_return(void(), svgIt == ScalableVectorGraphic::loadedSvgs.end());

    if (svgIt->second.use_count() <= 1)
        ScalableVectorGraphic::loadedSvgs.erase(svgIt);
}

void ScalableVectorGraphic::renderOffload(ssz renderIndex)
{
    if (this->dirty)
    {
        if (this->_svgFile != nullptr)
        {
            std::shared_ptr<std::vector<FilledPathRenderer>> swapToRender = this->findOrCreateRenderablePath(*this->_svgFile);

            std::lock_guard guard(this->renderData->toRenderLock);
            this->renderData->toRender = std::move(swapToRender);
        }
        if (this->deferOldSvg)
            this->buryLoadedSvgIfOrphaned(this->deferOldSvg);

        this->deferOldSvg = this->_svgFile.get();
        this->dirty = false;
    }

    CoreEngine::queueRenderJobForFrame([renderIndex, renderData = this->renderData, rectTransform = renderTransformFromRectTransform(rectTransform.get())]
    {
        std::lock_guard guard(renderData->toRenderLock);
        for (auto& paths : *renderData->toRender) (void)paths.submitDrawStencil(renderIndex, GL::RenderTransform());
        (void)FilledPathRenderer::submitDraw(renderIndex, rectTransform);
    }, false);
}
