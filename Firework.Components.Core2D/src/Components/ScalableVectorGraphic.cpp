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
            float alternatePtCtrl = 1.0f;

            std::vector<FringePoint> fringePoints;
            std::vector<ssz> fringePaths { 0_z };
            std::vector<glm::vec2> thisFringe;

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
                    filledPathPoints.emplace_back(ShapePoint { .x = pc.moveTo.to.x, .y = pc.moveTo.to.y, .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });
                    fringePoints.emplace_back(FringePoint { .x = pc.moveTo.to.x, .y = pc.moveTo.to.y });
                    break;
                case VectorTools::PathCommandType::LineTo:
                    pc.lineTo.to = transformByMatrix(pc.lineTo.to, childTransform);
                    filledPathPoints.emplace_back(ShapePoint { .x = pc.lineTo.to.x, .y = pc.lineTo.to.y, .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });
                    fringePoints.emplace_back(FringePoint { .x = pc.lineTo.to.x, .y = pc.lineTo.to.y });
                    break;
                case VectorTools::PathCommandType::CubicTo:
                    {
                        VectorTools::QuadApproxCubic converted = VectorTools::cubicBeizerToQuadratic(pc.cubicTo.from, pc.cubicTo.ctrl1, pc.cubicTo.ctrl2, pc.cubicTo.to);
                        converted.c1 = transformByMatrix(converted.c1, childTransform);
                        converted.p2 = transformByMatrix(converted.p2, childTransform);
                        converted.c2 = transformByMatrix(converted.c2, childTransform);
                        converted.p3 = transformByMatrix(converted.p3, childTransform);

                        filledPathPoints.emplace_back(ShapePoint { .x = converted.c1.x, .y = converted.c1.y, .xCtrl = 0.0f, .yCtrl = -1.0f });
                        filledPathPoints.emplace_back(ShapePoint { .x = converted.p2.x, .y = converted.p2.y, .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });
                        filledPathPoints.emplace_back(ShapePoint { .x = converted.c2.x, .y = converted.c2.y, .xCtrl = 0.0f, .yCtrl = -1.0f });
                        filledPathPoints.emplace_back(ShapePoint { .x = converted.p3.x, .y = converted.p3.y, .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });

                        if (VectorTools::quadraticBezierToLines(converted.p1, converted.c1, converted.p2, 32.0f, thisFringe) && !thisFringe.empty())
                            thisFringe.pop_back();
                        (void)VectorTools::quadraticBezierToLines(converted.p2, converted.c2, converted.p3, 32.0f, thisFringe);
                        if (!thisFringe.empty())
                            std::transform(++thisFringe.cbegin(), thisFringe.cend(), std::back_inserter(fringePoints),
                                           [](glm::vec2 v) { return FringePoint { .x = v.x, .y = v.y }; });
                        thisFringe.clear();
                    }
                    break;
                case VectorTools::PathCommandType::QuadraticTo:
                    {
                        pc.quadraticTo.ctrl = transformByMatrix(pc.quadraticTo.ctrl, childTransform);
                        pc.quadraticTo.to = transformByMatrix(pc.quadraticTo.to, childTransform);

                        const glm::vec2& from = pc.quadraticTo.from;
                        const glm::vec2& ctrl = pc.quadraticTo.ctrl;
                        const glm::vec2& to = pc.quadraticTo.to;

                        filledPathPoints.emplace_back(ShapePoint { .x = ctrl.x, .y = ctrl.y, .xCtrl = 0.0f, .yCtrl = -1.0f });
                        filledPathPoints.emplace_back(ShapePoint { .x = to.x, .y = to.y, .xCtrl = (alternatePtCtrl = -alternatePtCtrl), .yCtrl = 1.0f });

                        (void)VectorTools::quadraticBezierToLines(from, ctrl, to, 32.0f, thisFringe);
                        if (!thisFringe.empty())
                            std::transform(++thisFringe.cbegin(), thisFringe.cend(), std::back_inserter(fringePoints),
                                           [](glm::vec2 v) { return FringePoint { .x = v.x, .y = v.y }; });
                        thisFringe.clear();
                    }
                    break;
                case VectorTools::PathCommandType::ClosePath:
                    filledPaths.emplace_back(ssz(filledPathPoints.size()));
                    fringePaths.emplace_back(ssz(fringePoints.size()));
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
                (void)renderable.fringe.rend.submitDraw(float(+renderIndex), renderData->tf);
                break;
            case ScalableVectorGraphic::RenderableType::NoOp:;
            }
        }
    }, false);
}
