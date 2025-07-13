#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <stb_image.h>

#include <Core/PackageManager.h>

namespace Firework::PackageSystem
{
    class _fw_cc2d_api PortableGraphicPackageFile final : public PackageFile
    {
        stbi_uc* loadedImage;
        int width, height;
    public:
        PortableGraphicPackageFile(std::vector<uint8_t>&& data)
        {
            int channels;
            this->loadedImage = stbi_load_from_memory(data.data(), data.size(), &this->width, &this->height, &channels, 4);
        }
        ~PortableGraphicPackageFile() override
        {
            stbi_image_free(this->loadedImage);
        }

        /// @brief Retrieve the width of the image.
        /// @return The width of the image, in pixels.
        int imageWidth()
        {
            return this->width;
        };
        /// @brief Retrieve the height of the image.
        /// @return The height of the image, in pixels.
        int imageHeight()
        {
            return this->height;
        };
        /// @brief Retrieve the byte data of the image.
        /// @return Contiguous byte array of color data. Column major indexing.
        const uint8_t* imageRGBAData()
        {
            return this->loadedImage;
        };

        friend class Firework::PackageSystem::PackageManager;
    };
} // namespace Firework::PackageSystem
