#include "ScalableVectorGraphic.h"

#include <Friends/PackageFileCore2D.h>

using namespace Firework;
using namespace Firework::PackageSystem;

std::shared_ptr<std::vector<FilledPathRenderer>> ScalableVectorGraphic::findOrCreateRenderablePath(PackageSystem::ExtensibleMarkupPackageFile* svg)
{
    auto svgIt = ScalableVectorGraphic::loadedSvgs.find(svg);
    _fence_value_return(svgIt->second, svgIt != ScalableVectorGraphic::loadedSvgs.end());

    _fence_value_return(nullptr, !svg);

    std::vector<FilledPathRenderer> ret;
    sysm::vector2 viewBox = sysm::vector2(100.0f);

    auto setViewBox = [&](pugi::xml_node node)
    {
        if (pugi::xml_attribute vbAttr = node.attribute("viewBox"))
        {
            _fence_value_return(void(), !vbAttr);

            std::istringstream iss(vbAttr.as_string());
            float a, b;
            _fence_value_return(void(), !(iss >> a));
            _fence_value_return(void(), !(iss >> b));

            _fence_value_return(void(), !(iss >> a));
            _fence_value_return(void(), iss >> b); // Must end at fourth number.

            viewBox.x = a;
            viewBox.y = b;
        }
        else if (pugi::xml_attribute wAttr = node.attribute("width"), hAttr = node.attribute("height"); wAttr && hAttr)
        {
            float w = wAttr.as_float(std::numeric_limits<float>::infinity()), h = hAttr.as_float(std::numeric_limits<float>::infinity());
            _fence_value_return(void(), std::isinf(w) || std::isinf(h));

            viewBox.x = w;
            viewBox.y = h;
        }
    };
    auto recurse = [&](auto&& recurse, pugi::xml_node front, std::string_view fill) -> void
    {
        _fence_value_return(void(), !parent);

        for (pugi::xml_node node : front)
        {
            pugi::xml_attribute fAttr = node.attribute("fill");
            std::string_view newFill = fAttr ? fAttr.as_string() : fill;
            if (std::strcmp(node.name(), "path") == 0)
            {
            }
            else if (std::strcmp(node.name(), "g") == 0)
                recurse(recurse, node, newFill);
            else if (std::strcmp(node.name(), "svg") == 0)
            {
                setViewBox(node);
                recurse(recurse, node, newFill);
            }
        }
    };
    recurse(recurse, svg->document().root(), "none");
}
void ScalableVectorGraphic::buryLoadedSvgIfOrphaned(PackageSystem::ExtensibleMarkupPackageFile* svg)
{
    auto svgIt = ScalableVectorGraphic::loadedSvgs.find(svg);
    _fence_value_return(void(), svgIt == ScalableVectorGraphic::loadedSvgs.end());

    if (svgIt->second.use_count() <= 1)
        ScalableVectorGraphic::loadedSvgs.erase(svgIt);
}
