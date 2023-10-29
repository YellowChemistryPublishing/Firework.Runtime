#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <robin_hood.h>

#include <Components/PackageFileCore.h>
#include <Components/RectTransform.h>
#include <Core/PackageManager.h>
#include <Drawing/Core.h>
#include <GL/Renderer.h>
#include <Objects/Component2D.h>
#include <Library/Property.h>

namespace Firework
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;
    }

    class __firework_componentcore2d_api Image final : public Internal::Component2D
    {
        static GL::GeometryProgramHandle program;

        static void renderInitialize();

        struct TextureData
        {
            uint32_t accessCount;
            GL::Texture2DHandle internalTexture;
            GL::TextureSamplerHandle internalSampler;
        };
        static robin_hood::unordered_map<PackageSystem::PortableGraphicPackageFile*, TextureData*> imageTextures;
        TextureData* textureData = nullptr;

        struct RenderData
        {
            GL::DynamicMeshHandle internalMesh;
        };
        RenderData* data = nullptr;

        void renderOffload();

        // Rendering ^ / v Data

        RectFloat split { 0.0f, 0.0f, 0.0f, 0.0f };
        PackageSystem::PortableGraphicPackageFile* file = nullptr;

        void updateImageSplit();
        void setImageSplit(const RectFloat& value);
        void setImageFile(PackageSystem::PortableGraphicPackageFile* value);
    public:
        ~Image() override;

        const Property<const RectFloat&, const RectFloat&> imageSplit
        {{
            [this]() -> const RectFloat& { return this->split; },
            [this](const RectFloat& value) { this->setImageSplit(value); }
        }};
        const Property<PackageSystem::PortableGraphicPackageFile*, PackageSystem::PortableGraphicPackageFile*> imageFile
        {{
            [this]() -> PackageSystem::PortableGraphicPackageFile* { return this->file; },
            [this](PackageSystem::PortableGraphicPackageFile* value) { this->setImageFile(value); }
        }};
        Color tint { 255, 255, 255, 255 };

        friend struct Firework::Internal::ComponentCoreStaticInit;
    };
}