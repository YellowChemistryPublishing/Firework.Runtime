#include "ScalableVectorGraphic.h"

#include <Components/RectTransform.h>
#include <Core/CoreEngine.h>
#include <Core/Debug.h>
#include <EntityComponentSystem/Entity.h>
#include <Friends/VectorTools.h>
#include <GL/Renderer.h>
#include <Library/Math.h>
#include <PackageSystem/ExtensibleMarkupFile.h>

using namespace Firework;
using namespace Firework::GL;
using namespace Firework::Internal;
using namespace Firework::PackageSystem;

robin_hood::unordered_map<PackageSystem::ExtensibleMarkupPackageFile*, std::shared_ptr<std::vector<ScalableVectorGraphic::Renderable>>> ScalableVectorGraphic::loadedSvgs;

void ScalableVectorGraphic::onAttach(Entity& entity)
{
    this->rectTransform = entity.getOrAddComponent<RectTransform>();
}

std::shared_ptr<std::vector<ScalableVectorGraphic::Renderable>> ScalableVectorGraphic::findOrCreateRenderablePath(PackageSystem::ExtensibleMarkupPackageFile& svg)
{
    const auto svgIt = ScalableVectorGraphic::loadedSvgs.find(&svg);
    _fence_value_return(svgIt->second, svgIt != ScalableVectorGraphic::loadedSvgs.end());

    std::vector<Renderable> ret;

    const auto traverse = [&](const auto& traverse, const pugi::xml_node& node, glm::mat3x3 transform = glm::mat3x3(1.0f), Color fill = Color::unknown) -> void
    {
        sys::result<Color> cfRes = VectorTools::parseColor(node.attribute("fill").value());
        Color childFill = cfRes ? cfRes.move() : fill;

        sys::result<glm::mat3x3> ctRes = VectorTools::parseTransform(node.attribute("transform").value());
        glm::mat3x3 childTransform = ctRes ? ctRes.move() * transform : transform;

        std::basic_string_view<pugi::char_t> tag = node.name();
        if (tag == PUGIXML_TEXT("svg"))
        {
            for (const pugi::xml_node& child : node) traverse(traverse, child, childTransform, childFill);
        }
        else if (tag == PUGIXML_TEXT("g"))
        {
            for (const pugi::xml_node& child : node) traverse(traverse, child, childTransform, childFill);
        }
        else if (tag == PUGIXML_TEXT("path"))
        {
            std::vector<VectorTools::PathCommand> pathCommands;
            _fence_value_return(void(), !VectorTools::parsePath(node.attribute("d").value(), pathCommands));

            std::vector<ShapePoint> filledPathPoints;
            std::vector<ssz> filledPaths { 0_z };
            std::vector<FringePoint> fringePoints;
            std::vector<ssz> fringePaths { 0_z };
            std::vector<ShapeOutlinePoint> currentPath;

            constexpr auto transformByMatrix = [](glm::vec2 point, glm::mat3x3 transform) -> glm::vec2
            {
                glm::vec3 ret = transform * glm::vec3(point.x, point.y, 1.0f);
                return glm::vec2(ret.x, ret.y);
            };

            for (VectorTools::PathCommand& pc : pathCommands)
            {
                switch (pc.type)
                {
                case VectorTools::PathCommandType::MoveTo:
                    pc.moveTo.to = transformByMatrix(pc.moveTo.to, childTransform);
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.moveTo.to.x, .y = pc.moveTo.to.y, .isCtrl = false });
                    break;
                case VectorTools::PathCommandType::LineTo:
                    pc.lineTo.to = transformByMatrix(pc.lineTo.to, childTransform);
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.lineTo.to.x, .y = pc.lineTo.to.y, .isCtrl = false });
                    break;
                case VectorTools::PathCommandType::CubicTo:
                    pc.cubicTo.ctrl1 = transformByMatrix(pc.cubicTo.ctrl1, childTransform);
                    pc.cubicTo.ctrl2 = transformByMatrix(pc.cubicTo.ctrl2, childTransform);
                    pc.cubicTo.to = transformByMatrix(pc.cubicTo.to, childTransform);
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.cubicTo.ctrl1.x, .y = pc.cubicTo.ctrl1.y, .isCtrl = true });
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.cubicTo.ctrl2.x, .y = pc.cubicTo.ctrl2.y, .isCtrl = true });
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.cubicTo.to.x, .y = pc.cubicTo.to.y, .isCtrl = false });
                    break;
                case VectorTools::PathCommandType::QuadraticTo:
                    pc.quadraticTo.ctrl = transformByMatrix(pc.quadraticTo.ctrl, childTransform);
                    pc.quadraticTo.to = transformByMatrix(pc.quadraticTo.to, childTransform);
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.quadraticTo.ctrl.x, .y = pc.quadraticTo.ctrl.y, .isCtrl = true });
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.quadraticTo.to.x, .y = pc.quadraticTo.to.y, .isCtrl = false });
                    break;
                case VectorTools::PathCommandType::ClosePath:
                    if (currentPath.empty())
                        break;

                    (void)VectorTools::shapeFromOutline(currentPath, filledPathPoints);
                    filledPaths.emplace_back(filledPathPoints.size());
                    (void)VectorTools::fringeFromOutline(currentPath, fringePoints);
                    fringePaths.emplace_back(fringePoints.size());
                    currentPath.clear();
                    break;
                case VectorTools::PathCommandType::ArcTo: /* TODO */;
                }
            }

            if (FringeRenderer fr = FringeRenderer(fringePoints, fringePaths))
                ret.emplace_back(FringeRenderable(std::move(fr), childFill));
            if (ShapeRenderer pr = ShapeRenderer(filledPathPoints, filledPaths))
                ret.emplace_back(FilledPathRenderable(std::move(pr), childFill));
        }
        // Otherwise, do nothing.
    };
    for (const pugi::xml_node& node : *this->_svgFile) traverse(traverse, node);

    _fence_value_return(nullptr, ret.empty());

    std::shared_ptr<std::vector<ScalableVectorGraphic::Renderable>> ptr = std::make_shared<std::vector<ScalableVectorGraphic::Renderable>>(std::move(ret));
    ScalableVectorGraphic::loadedSvgs.emplace(&svg, ptr);
    return ptr;
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
    // Don't forget to lock the render data!
    auto setViewboxTransform = [&]
    {
        const RectTransform& transform = *this->rectTransform;
        const VectorTools::Viewbox& vb = this->renderData->vb;

        glm::mat4 ret = glm::translate(glm::mat4(1.0f), glm::vec3(transform.position().x, transform.position().y, 0.0f));
        ret = glm::rotate(ret, -transform.rotation(), LinAlgConstants::forward);
        ret = glm::translate(ret, glm::vec3((transform.rect().right + transform.rect().left) / 2.0f, (transform.rect().top + transform.rect().bottom) / 2.0f, 0.0f));

        float scFactor = std::min(transform.rect().width() / vb.w, transform.rect().height() / vb.h);
        ret = glm::scale(ret, glm::vec3(scFactor * transform.scale().x, -scFactor * transform.scale().y, 1.0f));
        ret = glm::translate(ret, glm::vec3(-vb.x - vb.w * 0.5f, -vb.y - vb.h * 0.5f, 0.0f));

        this->renderData->tf = ret;
    };

    if (this->dirty)
    {
        if (this->_svgFile != nullptr)
        {
            std::shared_ptr<std::vector<ScalableVectorGraphic::Renderable>> swapToRender = this->findOrCreateRenderablePath(*this->_svgFile);
            if (swapToRender)
            {
                sys::result<VectorTools::Viewbox> vb = VectorTools::parseViewbox(this->_svgFile->document().child(PUGIXML_TEXT("svg")).attribute(PUGIXML_TEXT("viewBox")).value());

                std::lock_guard guard(this->renderData->lock);
                this->renderData->toRender = std::move(swapToRender);
                if (vb)
                    this->renderData->vb = vb.move();
                else
                    this->renderData->vb = VectorTools::Viewbox { 0.0f, 0.0f, 100.0f, 100.0f };
                setViewboxTransform();
            }
        }
        else
        {
            std::lock_guard guard(this->renderData->lock);
            this->renderData->toRender = nullptr;
        }
        if (this->deferOldSvg)
            this->buryLoadedSvgIfOrphaned(this->deferOldSvg);

        this->deferOldSvg = this->_svgFile.get();
        this->dirty = false;
    }
    else if (this->rectTransform->dirty())
    {
        std::lock_guard guard(this->renderData->lock);
        setViewboxTransform();
    }

    CoreEngine::queueRenderJobForFrame([renderIndex, renderData = this->renderData, rectTransform = rectTransform->matrix()]
    {
        std::lock_guard guard(renderData->lock);
        _fence_value_return(void(), !renderData->toRender);

        for (auto it = renderData->toRender->rbegin(); it != renderData->toRender->rend(); ++it)
        {
            const ScalableVectorGraphic::Renderable& renderable = *it;
            switch (renderable.type)
            {
            case ScalableVectorGraphic::RenderableType::FilledPath:
                (void)renderable.filledPath.rend.submitDrawStencil(float(+renderIndex), renderData->tf);
                (void)ShapeRenderer::submitDraw(float(+renderIndex), rectTransform, ~0_u8, renderable.filledPath.col);
                break;
            case ScalableVectorGraphic::RenderableType::Fringe:
                (void)renderable.fringe.rend.submitDraw(float(+renderIndex), renderData->tf, renderable.fringe.col);
                break;
            case ScalableVectorGraphic::RenderableType::NoOp:;
            }
        }
    }, false);
}
