$input v_texcoord0, v_color0

#include "bgfx_shader.sh"

uniform vec4 u_params;

#define u_stage u_params.x

void main()
{
    vec2 px = dFdx(v_texcoord0);
    vec2 py = dFdy(v_texcoord0);
    
    if (u_stage == 0.0)
    {
        if (v_texcoord0.y - v_texcoord0.x * v_texcoord0.x <= 0.0)
            discard;
    }
    else if (u_stage == 1.0)
    {
        gl_FragColor = v_color0;
    }
    else
    {
        float fx = (2.0 * v_texcoord0.x) * px.x - px.y;
        float fy = (2.0 * v_texcoord0.x) * py.x - py.y;
        
        float sd = (v_texcoord0.y - v_texcoord0.x * v_texcoord0.x) / sqrt(fx * fx + fy * fy);
        float alpha = saturate(abs(sd));
        
        if (alpha >= 1.0)
            discard;
        if (alpha <= 0.0)
            discard;
            
        gl_FragColor = vec4(v_color0.rgb, v_color0.a * (1.0 - alpha));
    }
}
