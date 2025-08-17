$input v_texcoord0, v_color0

#include "bgfx_shader.sh"

uniform vec4 u_params;

#define u_alphaFract u_params.y

void main()
{
    if (v_texcoord0.y - v_texcoord0.x * v_texcoord0.x <= 0.0)
        discard;
    gl_FragColor = vec4(v_color0.rgb, v_color0.a * u_alphaFract);
}
