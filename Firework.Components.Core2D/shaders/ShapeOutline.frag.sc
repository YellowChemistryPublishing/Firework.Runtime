$input v_texcoord0, v_color0

#include "bgfx_shader.sh"

uniform vec4 u_params;

#define PATH_TYPE_FILL_STENCIL 0.0
#define PATH_TYPE_FILL_COVER 1.0
#define PATH_TYPE_LINE_FRINGE 2.0
#define PATH_TYPE_QUADRATIC_FRINGE 3.0

#define u_pathType u_params.x
#define u_alphaFract u_params.y

void main()
{
    if (u_pathType == PATH_TYPE_FILL_STENCIL)
    {
        if (v_texcoord0.y - v_texcoord0.x * v_texcoord0.x <= 0.0)
            discard;
    }
    else if (u_pathType == PATH_TYPE_FILL_COVER || u_pathType == PATH_TYPE_LINE_FRINGE)
        gl_FragColor = vec4(v_color0.rgb, v_color0.a * u_alphaFract);
    else if (u_pathType == PATH_TYPE_QUADRATIC_FRINGE)
    {
        float f = v_texcoord0.y - v_texcoord0.x * v_texcoord0.x;
        vec2 df = vec2(dFdx(f), dFdy(f));
        float sd = f / length(df);
        
        float alpha = 1.0 - 0.5 * abs(sd);
        if (alpha <= 0.0)
            discard;
        alpha = saturate(alpha);

        gl_FragColor = vec4(v_color0.rgb, alpha * v_color0.a);
    }
    else discard;
}
