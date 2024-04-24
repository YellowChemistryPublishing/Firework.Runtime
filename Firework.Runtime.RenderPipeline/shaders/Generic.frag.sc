$input v_position, v_normal, v_color0

#include "bgfx_shader.sh" 

uniform vec4 u_ambientData;
#define u_ambient u_ambientData.x

SAMPLER2D(s_directionalLights, 0);

float diffuse(vec3 incident, vec3 normal)
{
    return dot(incident, normal);
}

void main()
{
    gl_FragColor = vec4((diffuse(normalize(texelFetch(s_directionalLights, ivec2(0, 0), 0).xyz), v_normal) + u_ambient) * texelFetch(s_directionalLights, ivec2(1, 0), 0).xyz * v_color0.xyz, 1.0);
}
