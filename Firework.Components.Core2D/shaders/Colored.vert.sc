$input a_position
$output v_color0

#include "bgfx_shader.sh"

uniform vec4 u_color;

void main()
{
    gl_Position = mul(u_modelViewProj, a_position);
    v_color0 = u_color;
}