$input v_texcoord0

#include "bgfx_shader.sh"

uniform vec4 u_color;

void main()
{
    if (v_texcoord0.y < v_texcoord0.x * v_texcoord0.x)
        discard;

    gl_FragColor = u_color;
}