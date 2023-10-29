$input v_pos, v_normal, v_texcoord0

#include "bgfx_shader.sh"
#include "../include/GL.Config.h"
#include "Lighting.sh"

uniform mat4 u_directionalLights[GL_MAX_DIRECTIONAL_LIGHT_COUNT];
uniform mat4 u_pointLights[GL_MAX_PROXIMAL_POINT_LIGHT_COUNT];

uniform mat4 u_material;

SAMPLER2D(s_texColor, 0);

void main()
{
    vec3 normalizedNormal = normalize(v_normal);
    vec3 viewDir = normalize(mul(vec4(0.0, 0.0, 0.0, 0.0), u_invView).xyz - v_pos);
    vec4 color = texture2D(s_texColor, v_texcoord0.xy);

    vec3 fragOut = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < GL_MAX_DIRECTIONAL_LIGHT_COUNT; i++)
        fragOut += calcDirLight(u_material, u_directionalLights[i], normalizedNormal, viewDir, color.xyz, color.xyz);
    for (int i = 0; i < GL_MAX_PROXIMAL_POINT_LIGHT_COUNT; i++)
        fragOut += calcPointLight(u_material, u_pointLights[i], normalizedNormal, viewDir, v_pos, color.xyz, color.xyz);
    gl_FragColor = vec4(fragOut, 1.0);
}