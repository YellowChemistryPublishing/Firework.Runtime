$input a_position, a_texcoord0
$output v_texcoord0, v_color0

#include "bgfx_shader.sh"

uniform vec4 u_color;

void main()
{
    v_texcoord0 = a_texcoord0;
    v_color0 = u_color;
    gl_Position = mul(u_modelViewProj, a_position);
}
