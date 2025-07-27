$input v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(s_frame, 0);

void main()
{
    vec4 col = texture2D(s_frame, v_texcoord0);
    if (col.w == 0.0)
        discard;

    gl_FragColor = col;
}
