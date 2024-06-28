$input v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(s_imageTexture, 0);
uniform vec4 u_tint;

void main()
{
    vec4 color = texture2D(s_imageTexture, v_texcoord0) * u_tint;
    gl_FragColor = color;
}