$input a_position, a_texcoord0
$output v_texcoord0

#include "bgfx_shader.sh"

void main()
{
    gl_Position = mul(u_modelViewProj, a_position);
    v_texcoord0 = a_texcoord0;
}