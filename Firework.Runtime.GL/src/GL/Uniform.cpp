#include "Uniform.h"

using namespace Firework::GL;

UniformHandle UniformHandle::create(const char* name, UniformType type)
{
    UniformHandle ret;
    ret.internalHandle = bgfx::createUniform(name, (bgfx::UniformType::Enum)type, 1);
    return ret;
}
UniformHandle UniformHandle::createArray(const char* name, UniformType type, uint16_t count)
{
    UniformHandle ret;
    ret.internalHandle = bgfx::createUniform(name, (bgfx::UniformType::Enum)type, count);
    return ret;
}
void UniformHandle::destroy()
{
    bgfx::destroy(this->internalHandle);
}

UniformHandle::operator bool () const
{
    return bgfx::isValid(this->internalHandle);
}