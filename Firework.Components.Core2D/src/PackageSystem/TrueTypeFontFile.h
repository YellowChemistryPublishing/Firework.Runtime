#pragma once

#include "Firework.Components.Core2D.Exports.h"

#include <Core/PackageManager.h>
#include <Font/Font.h>

_push_nowarn_msvc(_clWarn_msvc_export_interface);
namespace Firework::PackageSystem
{
    class _fw_cc2d_api TrueTypeFontPackageFile final : public PackageFile
    {
        std::vector<byte> data;
        Typography::Font font;
    public:
        TrueTypeFontPackageFile(std::vector<uint8_t>&& data) : data(std::move(data)), font(_asr(unsigned char*, this->data.data()))
        { }

        Typography::Font& fontHandle()
        {
            return this->font;
        }

        friend class Firework::PackageSystem::PackageManager;
    };
} // namespace Firework::PackageSystem
_pop_nowarn_msvc();
