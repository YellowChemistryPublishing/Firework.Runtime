#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <CompilerWarnings.h>

_push_nowarn_clang(_clWarn_clang_zero_as_nullptr);
#include <bgfx/bgfx.h>
#include <cstddef>
#include <glm/mat4x4.hpp>
#include <module/sys>
#include <utility>
_pop_nowarn_clang();

#include <GL/Common.h>
#include <GL/TextureFormat.h>

namespace Firework::GL
{
    template <typename... Ts>
    requires (std::same_as<Ts, std::remove_cvref_t<Ts>> && ...) && ((sizeof(float) * 16 + sizeof(Ts) + ...) == sizeof(InstanceData<Ts...>))
    struct _packed InstanceData final
    {
        glm::mat4 transform = glm::mat4(1.0f);
        std::tuple<Ts...> data;
    };
} // namespace Firework::GL
