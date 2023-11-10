#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <stb_image.h>
#include <Core/PackageManager.h>
#include <Font/Font.h>

namespace Firework
{
    namespace PackageSystem
    {
        class __firework_componentcore2d_api PortableGraphicPackageFile final : public PackageFile
        {
            stbi_uc* loadedImage;
            int width, height;
            
            inline PortableGraphicPackageFile(std::vector<uint8_t> data)
            {
                int channels;
                this->loadedImage = stbi_load_from_memory(data.data(), data.size(), &this->width, &this->height, &channels, 4);
            }
            inline ~PortableGraphicPackageFile() override
            {
                stbi_image_free(this->loadedImage);
            }
        public:
            /// @brief Retrieve the width of the image.
            /// @return The width of the image, in pixels.
            inline int imageWidth() { return this->width; };
            /// @brief Retrieve the height of the image.
            /// @return The height of the image, in pixels.
            inline int imageHeight() { return this->height; };
            /// @brief Retrieve the byte data of the image.
            /// @return Contiguous byte array of color data. Indexed (row? column? who knows FIXME:) major.
            inline const uint8_t* imageRGBAData() { return this->loadedImage; };

            friend class Firework::PackageSystem::PackageManager;
        };

        class __firework_componentcore2d_api TrueTypeFontPackageFile final : public PackageFile
        {
            std::vector<uint8_t> data;
            Typography::Font font;
            
            inline TrueTypeFontPackageFile(std::vector<uint8_t> data) : data(std::move(data)), font((unsigned char*)this->data.data())
            {
            }
        public:
            inline Typography::Font& fontHandle()
            {
                return this->font;
            }

            friend class Firework::PackageSystem::PackageManager;
        };
    }
}