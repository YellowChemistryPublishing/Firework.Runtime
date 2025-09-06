#include "ScalableVectorGraphic.h"

#include <Components/RectTransform.h>
#include <Core/CoreEngine.h>
#include <Core/Debug.h>
#include <EntityComponentSystem/Entity.h>
#include <Friends/CascadingStyleParser.h>
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
        std::map<std::string_view, std::string_view> style;
        (void)CascadingStyleParser::parseBlock(node.attribute("style").value(), style);

        const auto cfStyleIt = style.find("fill");
        sys::result<Color> cfRes = cfStyleIt != style.end() ? VectorParser::parseColor(cfStyleIt->second) : VectorParser::parseColor(node.attribute("fill").value());
        Color childFill = cfRes ? cfRes.move() : fill;

        sys::result<glm::mat3x3> ctRes = VectorParser::parseTransform(node.attribute("transform").value());
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
            std::vector<VectorParser::PathCommand> pathCommands;
            _fence_value_return(void(), !VectorParser::parsePath(node.attribute("d").value(), pathCommands));

            std::vector<ShapePoint> shapePoints;
            std::vector<uint16_t> shapeInds;
            std::vector<ShapePoint> shapeCurvePoints;
            std::vector<uint16_t> shapeCurveInds;
            std::vector<ShapeOutlinePoint> currentPath;

            constexpr auto transformByMatrix = [](glm::vec2 point, glm::mat3x3 transform) -> glm::vec2
            {
                glm::vec3 ret = transform * glm::vec3(point.x, point.y, 1.0f);
                return glm::vec2(ret.x, ret.y);
            };

            glm::vec2 windAround(0.0f, 0.0f);
            sz windCount = 0;
            glm::vec2 min(std::numeric_limits<float>::infinity()), max(-std::numeric_limits<float>::infinity());
            for (VectorParser::PathCommand& pc : pathCommands)
            {
                switch (pc.type)
                {
                case VectorParser::PathCommandType::CubicTo:
                    pc.dc.ctrl1 = transformByMatrix(pc.dc.ctrl1, childTransform);
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.dc.ctrl1.x, .y = pc.dc.ctrl1.y, .isCtrl = true });
                    min = glm::min(min, pc.dc.ctrl1);
                    max = glm::max(max, pc.dc.ctrl1);
                    [[fallthrough]];
                case VectorParser::PathCommandType::QuadraticTo:
                    pc.dc.ctrl2 = transformByMatrix(pc.dc.ctrl2, childTransform);
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.dc.ctrl2.x, .y = pc.dc.ctrl2.y, .isCtrl = true });
                    min = glm::min(min, pc.dc.ctrl2);
                    max = glm::max(max, pc.dc.ctrl2);
                    [[fallthrough]];
                case VectorParser::PathCommandType::MoveTo:
                case VectorParser::PathCommandType::LineTo:
                    pc.to = transformByMatrix(pc.to, childTransform);
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.to.x, .y = pc.to.y, .isCtrl = false });
                    min = glm::min(min, pc.to);
                    max = glm::max(max, pc.to);
                    windAround += pc.to;
                    ++windCount;
                    break;
                case VectorParser::PathCommandType::ClosePath:
                    if (currentPath.empty())
                        break;

                    windAround /= float(+windCount);
                    (void)VectorTools::shapeTrianglesFromOutline(currentPath, shapePoints, shapeInds, windAround);
                    (void)VectorTools::shapeProcessCurvesFromOutline(currentPath, shapeCurvePoints, shapeCurveInds, shapePoints, shapeInds);

                    currentPath.clear();
                    windAround = glm::vec2(0.0f, 0.0f);
                    windCount = 0;
                    break;
                case VectorParser::PathCommandType::ArcTo: /* TODO */;
                }
            }

            u32 curvePointsBegin = shapePoints.size();
            u32 curveIndsBegin = shapeInds.size();
            shapePoints.insert(shapePoints.end(), shapeCurvePoints.begin(), shapeCurvePoints.end());
            std::transform(shapeCurveInds.begin(), shapeCurveInds.end(), std::back_inserter(shapeInds),
                           [curvePointsBegin](const uint16_t i) { return +(i + u16(curvePointsBegin)); });
            if (ShapeRenderer pr = ShapeRenderer(shapePoints, shapeInds, curveIndsBegin))
            {
                // Add `2.0f` on either side to not clip out AA.
                glm::mat4 clipTf = glm::translate(glm::mat4(1.0f), glm::vec3(min.x - 2.0f, min.y - 2.0f, 0.0f));
                clipTf = glm::scale(clipTf, glm::vec3(max.x - min.x + 4.0f, -(max.y - min.y + 4.0f), 1.0f));
                clipTf = glm::translate(clipTf, glm::vec3(0.5f, -0.5f, 0.0f));
                ret.emplace_back(FilledPathRenderable(std::move(pr), clipTf, childFill));
            }
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

void ScalableVectorGraphic::lateRenderOffload(const ssz renderIndex)
{
    // Don't forget to lock the render data!
    auto setViewboxTransform = [&]
    {
        const RectTransform& transform = *this->rectTransform;
        const VectorParser::Viewbox& vb = this->renderData->vb;

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
                sys::result<VectorParser::Viewbox> vb =
                    VectorParser::parseViewbox(this->_svgFile->document().child(PUGIXML_TEXT("svg")).attribute(PUGIXML_TEXT("viewBox")).value());

                std::lock_guard guard(this->renderData->lock);
                this->renderData->toRender = std::move(swapToRender);
                if (vb)
                    this->renderData->vb = vb.move();
                else
                    this->renderData->vb = VectorParser::Viewbox { 0.0f, 0.0f, 100.0f, 100.0f };
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

    CoreEngine::queueRenderJobForFrame([renderIndex, renderData = this->renderData]
    {
        std::lock_guard guard(renderData->lock);
        _fence_value_return(void(), !renderData->toRender);

        float riIncr = 1.0f;
        float ri = float(+renderIndex) + float(renderData->toRender->size()) - riIncr;
        for (const ScalableVectorGraphic::Renderable& renderable : *renderData->toRender)
        {
            switch (renderable.type)
            {
            case ScalableVectorGraphic::RenderableType::FilledPath:
                {
                    _push_nowarn_c_cast();

                    const glm::mat4 clipTf = renderData->tf * renderable.filledPath.tf;
                    (void)ShapeRenderer::submitDrawCover(ri, clipTf, 0_u8, Color(0, 0, 0, 0),
                                                         BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_ZERO), 0);
                    (void)renderable.filledPath.rend.submitDrawStencil(ri, renderData->tf, FillRule::NonZero);
                    (void)ShapeRenderer::submitDrawCover(
                        ri, clipTf, 0_u8, renderable.filledPath.col,
                        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS |
                            BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_DST_ALPHA, BGFX_STATE_BLEND_INV_DST_ALPHA, BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_ONE),
                        BGFX_STENCIL_TEST_NOTEQUAL | BGFX_STENCIL_OP_FAIL_S_REPLACE | BGFX_STENCIL_OP_PASS_Z_REPLACE);

                    _pop_nowarn_c_cast();
                }
                break;
            case ScalableVectorGraphic::RenderableType::NoOp:
            default:;
            }
            ri -= riIncr;
        }
    }, false);
}
