$input v_pos, v_normal, v_color0, v_texcoord0

#include "bgfx_shader.sh"

void main()
{
    gl_FragColor = vec4(v_normal, 1.0);
}