#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <pugixml.hpp>
#include <stb_image.h>

#include <Core/PackageManager.h>
#include <Font/Font.h>

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

    class _fw_cc2d_api TrueTypeFontPackageFile final : public PackageFile
    {
        std::vector<uint8_t> data;
        Typography::Font font;
    public:
        TrueTypeFontPackageFile(std::vector<uint8_t>&& data) : data(std::move(data)), font((unsigned char*)this->data.data())
        { }

        Typography::Font& fontHandle()
        {
            return this->font;
        }

        friend class Firework::PackageSystem::PackageManager;
    };

    class _fw_cc2d_api ExtensibleMarkupPackageFile final : public PackageFile
    {
        std::u8string buffer;
        pugi::xml_document doc;
        pugi::xml_parse_result ok;
    public:
        ExtensibleMarkupPackageFile(std::u8string&& contents) : buffer(std::move(contents))
        {
            this->ok = this->doc.load_buffer_inplace(this->buffer.data(), this->buffer.size() * sizeof(char8_t));
        }

        operator bool()
        {
            return this->ok;
        }

        const pugi::xml_document& document()
        {
            return this->doc;
        }
    };
} // namespace Firework::PackageSystem