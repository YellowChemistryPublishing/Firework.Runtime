#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <pugixml.hpp>

#include <Core/PackageManager.h>

namespace Firework::PackageSystem
{
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

        pugi::xml_node_iterator begin() const
        {
            return this->doc.begin();
        }
        pugi::xml_node_iterator end() const
        {
            return this->doc.end();
        }

        const pugi::xml_document& document()
        {
            return this->doc;
        }
    };
} // namespace Firework::PackageSystem
