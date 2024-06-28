$input v_position, v_normal, v_color0

#include "bgfx_shader.sh"

#include "../../Firework.Runtime.GL/src/GL/TextureVector.sh"

uniform vec4 u_ambientData;
#define u_ambient u_ambientData.x

TextureVector(s_directionalLights, 0);

float diffuse(vec3 incident, vec3 normal)
{
    return dot(incident, normal);
}

void main()
{
    gl_FragColor = vec4((diffuse(normalize(TextureVector_Index(s_directionalLights, 0, 0).xyz), v_normal) + u_ambient) * TextureVector_Index(s_directionalLights, 1, 0).xyz * v_color0.xyz, 1.0);
    // gl_FragColor = vec4(__texelFetch(ivec2(0, 0)).xyz, 1.0);
}
