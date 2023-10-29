$input v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(s_imageTexture, 0);
vec4 u_tint;

void main()
{
    gl_FragColor = texture2D(s_imageTexture, v_texcoord0) * u_tint;
}