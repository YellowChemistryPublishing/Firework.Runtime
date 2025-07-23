#include "Uniform.h"

using namespace Firework::GL;

Uniform::Uniform(const std::string_view name, const UniformType type, const u16 count)
{
    const std::string cName(name.begin(), name.end());
    this->internalHandle = bgfx::createUniform(cName.c_str(), _as(bgfx::UniformType::Enum, type), +count);
}
_fw_gl_common_handle_dtor(Uniform);
