#include "bgfx_shader.sh"

#define TextureVector(name, register) SAMPLER2D(name, register); uniform vec4 name##Data
#define TextureVector_Vec4Count(name) name##Data.x
#define TextureVector_Size(name) name##Data.y
#define TextureVector_Index(name, subindex, index) texture2D(name, vec2(float(subindex), float(index)) / vec2(max(1.0, name##Data.x), max(1.0, name##Data.y)))