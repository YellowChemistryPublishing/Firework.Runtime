$input a_position
$output v_pos

#include "bgfx_shader.sh"

void main()
{
    gl_Position = mul(u_modelViewProj, a_position);
    v_pos = mul(u_modelViewProj, a_position);
}