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

        /// @internal
        /// @brief Internal API. Initialization of image-related rendering.
        /// @note Main thread only.
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

        /// @internal
        /// @brief Internal API. Submit a draw call for this component.
        /// @note Main thread only.
        void renderOffload();

        // Rendering ^ / v Data

        RectFloat split { 0.0f, 0.0f, 0.0f, 0.0f };
        PackageSystem::PortableGraphicPackageFile* file = nullptr;

        /// @internal
        /// @brief Internal API. Updates the 9-square split of this image.
        /// @note Main thread only.
        void updateImageSplit();
        /// @internal
        /// @brief Internal API. Set the 9-square split of this image.
        /// @param value Split bounds to set.
        void setImageSplit(const RectFloat& value);
        /// @internal
        /// @brief Internal API. Set the image file displayed.
        /// @param value Image file to set.
        void setImageFile(PackageSystem::PortableGraphicPackageFile* value);
    public:
        ~Image() override;

        /// @property
        /// @brief [Property] The 9-square split of this image.
        /// @param value ```const RectFloat&```
        /// @return ```const RectFloat&```
        /// @note Main thread only.
        const Property<const RectFloat&, const RectFloat&> imageSplit
        {{
            [this]() -> const RectFloat& { return this->split; },
            [this](const RectFloat& value) { this->setImageSplit(value); }
        }};
        /// @property
        /// @brief [Property] The image file to display.
        /// @param value ```PackageSystem::PortableGraphicPackageFile*```
        /// @return ```PackageSystem::PortableGraphicPackageFile*```
        /// @note Main thread only.
        const Property<PackageSystem::PortableGraphicPackageFile*, PackageSystem::PortableGraphicPackageFile*> imageFile
        {{
            [this]() -> PackageSystem::PortableGraphicPackageFile* { return this->file; },
            [this](PackageSystem::PortableGraphicPackageFile* value) { this->setImageFile(value); }
        }};
        /// @brief The color tint of this image.
        /// @note Main thread only.
        Color tint { 255, 255, 255, 255 };

        friend struct Firework::Internal::ComponentCoreStaticInit;
    };
}