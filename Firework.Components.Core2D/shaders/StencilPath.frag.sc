$input v_texcoord0

#include "bgfx_shader.sh"

void main()
{
    if (v_texcoord0.y < v_texcoord0.x * v_texcoord0.x)
        discard;
}