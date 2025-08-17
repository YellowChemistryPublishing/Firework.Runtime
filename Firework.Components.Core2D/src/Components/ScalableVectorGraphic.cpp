#include "ScalableVectorGraphic.h"

#include <Components/RectTransform.h>
#include <Core/CoreEngine.h>
#include <Core/Debug.h>
#include <EntityComponentSystem/Entity.h>
#include <Friends/Multisample.h>
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

            std::vector<ShapePoint> shapePoints;
            std::vector<uint16_t> shapeInds;
            std::vector<ShapeOutlinePoint> currentPath;

            constexpr auto transformByMatrix = [](glm::vec2 point, glm::mat3x3 transform) -> glm::vec2
            {
                glm::vec3 ret = transform * glm::vec3(point.x, point.y, 1.0f);
                return glm::vec2(ret.x, ret.y);
            };

            glm::vec2 windAround(0.0f, 0.0f);
            sz windCount = 0;
            glm::vec2 min(std::numeric_limits<float>::infinity()), max(-std::numeric_limits<float>::infinity());
            for (VectorTools::PathCommand& pc : pathCommands)
            {
                switch (pc.type)
                {
                case VectorTools::PathCommandType::CubicTo:
                    pc.dc.ctrl1 = transformByMatrix(pc.dc.ctrl1, childTransform);
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.dc.ctrl1.x, .y = pc.dc.ctrl1.y, .isCtrl = true });
                    min = glm::min(min, pc.dc.ctrl1);
                    max = glm::max(max, pc.dc.ctrl1);
                    [[fallthrough]];
                case VectorTools::PathCommandType::QuadraticTo:
                    pc.dc.ctrl2 = transformByMatrix(pc.dc.ctrl2, childTransform);
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.dc.ctrl2.x, .y = pc.dc.ctrl2.y, .isCtrl = true });
                    min = glm::min(min, pc.dc.ctrl2);
                    max = glm::max(max, pc.dc.ctrl2);
                    [[fallthrough]];
                case VectorTools::PathCommandType::MoveTo:
                case VectorTools::PathCommandType::LineTo:
                    pc.to = transformByMatrix(pc.to, childTransform);
                    currentPath.emplace_back(ShapeOutlinePoint { .x = pc.to.x, .y = pc.to.y, .isCtrl = false });
                    min = glm::min(min, pc.to);
                    max = glm::max(max, pc.to);
                    windAround += pc.to;
                    ++windCount;
                    break;
                case VectorTools::PathCommandType::ClosePath:
                    if (currentPath.empty())
                        break;

                    windAround /= float(+windCount);
                    (void)VectorTools::shapeTrianglesFromOutline(currentPath, shapePoints, shapeInds, windAround);
                    (void)VectorTools::shapeProcessCurvesFromOutline(currentPath, shapePoints, shapeInds);

                    currentPath.clear();
                    windAround = glm::vec2(0.0f, 0.0f);
                    windCount = 0;
                    break;
                case VectorTools::PathCommandType::ArcTo: /* TODO */;
                }
            }

            if (ShapeRenderer pr = ShapeRenderer(shapePoints, shapeInds))
            {
                // Add `1.0f` on either side to not clip out AA.
                glm::mat4 clipTf = glm::translate(glm::mat4(1.0f), glm::vec3(min.x - 1.0f, min.y - 1.0f, 0.0f));
                clipTf = glm::scale(clipTf, glm::vec3(max.x - min.x + 2.0f, -(max.y - min.y + 2.0f), 1.0f));
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

void ScalableVectorGraphic::lateRenderOffload(ssz renderIndex)
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

    CoreEngine::queueRenderJobForFrame([renderIndex, renderData = this->renderData]
    {
        std::lock_guard guard(renderData->lock);
        _fence_value_return(void(), !renderData->toRender);

        for (const ScalableVectorGraphic::Renderable& renderable : *renderData->toRender)
        {
            switch (renderable.type)
            {
            case ScalableVectorGraphic::RenderableType::FilledPath:
                {
                    _push_nowarn_gcc(_clWarn_gcc_c_cast);
                    _push_nowarn_clang(_clWarn_clang_c_cast);

                    const glm::mat4 clipTf = renderData->tf * renderable.filledPath.tf;
                    (void)ShapeRenderer::submitDrawCover(float(+renderIndex), clipTf, 0_u8, Color(0, 0, 0, 0), 1.0f,
                                                         BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ZERO),
                                                         BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_OP_FAIL_S_REPLACE | BGFX_STENCIL_OP_PASS_Z_REPLACE);
                    for (const auto& [xOff, yOff] : multisampleOffsets)
                    {
                        (void)renderable.filledPath.rend.submitDrawStencil(float(+renderIndex), glm::translate(glm::mat4(1.0f), glm::vec3(xOff, yOff, 0.0f)) * renderData->tf,
                                                                           WindingRule::NonZero);
                        (void)ShapeRenderer::submitDrawCover(float(+renderIndex), clipTf, 0_u8, renderable.filledPath.col,
                                                             std::ceil(255.0f / float(multisampleOffsets.size())) / 255.0f,
                                                             BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ADD,
                                                             BGFX_STENCIL_TEST_NOTEQUAL | BGFX_STENCIL_OP_FAIL_S_REPLACE | BGFX_STENCIL_OP_PASS_Z_REPLACE);
                    }
                    (void)ShapeRenderer::submitDrawCover(
                        float(+renderIndex), clipTf, 0_u8, renderable.filledPath.col, 1.0f,
                        BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_LESS |
                            BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_DST_ALPHA, BGFX_STATE_BLEND_INV_DST_ALPHA, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ZERO),
                        BGFX_STENCIL_TEST_EQUAL | BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP);

                    _pop_nowarn_clang();
                    _pop_nowarn_gcc();
                }
                break;
            case ScalableVectorGraphic::RenderableType::NoOp:
            default:;
            }
        }
    }, false);
}
