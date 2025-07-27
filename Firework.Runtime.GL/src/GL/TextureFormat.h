#pragma once

#include <bgfx/bgfx.h>

namespace Firework::GL
{
    using TextureFormat = bgfx::TextureFormat::Enum;

    enum class BackbufferRatio
    {
        Equal = bgfx::BackbufferRatio::Equal,
        Half = bgfx::BackbufferRatio::Half,
        Quarter = bgfx::BackbufferRatio::Quarter,
        Eighth = bgfx::BackbufferRatio::Eighth,
        Sixteenth = bgfx::BackbufferRatio::Sixteenth,
        Double = bgfx::BackbufferRatio::Double
    };
} // namespace Firework::GL
