$input a_position, a_normal, a_texcoord0
$output v_pos, v_normal, v_texcoord0

#include "bgfx_shader.sh"

uniform mat3 u_normalMatrix;

void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_pos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    v_normal = mul(u_normalMatrix, a_normal);
    v_texcoord0 = a_texcoord0;
}