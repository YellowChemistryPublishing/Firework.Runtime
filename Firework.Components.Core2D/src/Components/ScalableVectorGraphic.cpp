#include "ScalableVectorGraphic.h"

#include <Components/RectTransform.h>
#include <Core/Debug.h>
#include <EntityComponentSystem/Entity.h>
#include <Friends/VectorTools.h>
#include <PackageSystem/ExtensibleMarkupFile.h>

using namespace Firework;
using namespace Firework::PackageSystem;

robin_hood::unordered_map<PackageSystem::ExtensibleMarkupPackageFile*, std::shared_ptr<std::vector<FilledPathRenderer>>> ScalableVectorGraphic::loadedSvgs;

void ScalableVectorGraphic::onAttach(Entity& entity)
{
    this->rectTransform = entity.getOrAddComponent<RectTransform>();
}

std::shared_ptr<std::vector<FilledPathRenderer>> ScalableVectorGraphic::findOrCreateRenderablePath(PackageSystem::ExtensibleMarkupPackageFile& svg)
{
    auto svgIt = ScalableVectorGraphic::loadedSvgs.find(&svg);
    _fence_value_return(svgIt->second, svgIt != ScalableVectorGraphic::loadedSvgs.end());

    std::vector<FilledPathRenderer> ret;
    sysm::vector2 viewBox = sysm::vector2(100.0f);

    std::string d = svg.document().root().first_child().first_child().find_child_by_attribute("id", "g50").first_child().attribute("d").value();
    std::vector<Firework::VectorTools::VectorPathCommand> pc;
    bool flag = VectorTools::parse(d.c_str(), pc);
    freopen("stdout.txt", "w", stdout);
    for (auto cmd : pc)
    {
        std::println("{}", (int)cmd.type);
    }

    return nullptr;
}
void ScalableVectorGraphic::buryLoadedSvgIfOrphaned(PackageSystem::ExtensibleMarkupPackageFile& svg)
{
    auto svgIt = ScalableVectorGraphic::loadedSvgs.find(&svg);
    _fence_value_return(void(), svgIt == ScalableVectorGraphic::loadedSvgs.end());

    if (svgIt->second.use_count() <= 1)
        ScalableVectorGraphic::loadedSvgs.erase(svgIt);
}

void ScalableVectorGraphic::renderOffload(ssz renderIndex)
{
    if (this->dirty)
    {
        if (this->deferOldSvg)
            this->buryLoadedSvgIfOrphaned(*this->deferOldSvg);
        if (this->_svgFile != nullptr)
            this->findOrCreateRenderablePath(*this->_svgFile);

        this->deferOldSvg = this->_svgFile.get();
        this->dirty = false;
    }
}
