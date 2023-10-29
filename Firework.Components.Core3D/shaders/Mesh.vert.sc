$input a_position, a_normal, a_color0, a_texcoord0
$output v_pos, v_normal, v_color0, v_texcoord0

#include "bgfx_shader.sh"

void main()
{
    v_pos = a_position;
    v_normal = a_normal;
    v_color0 = a_color0;
    v_texcoord0 = a_texcoord0;
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}