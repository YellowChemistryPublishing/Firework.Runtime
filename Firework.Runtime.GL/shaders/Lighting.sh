#ifndef LIGHTING_H_HEADER_GUARD
#define LIGHTING_H_HEADER_GUARD

#define dirLightNormNegDir(mat) mat[0].xyz
#define dirLightAmbient(mat) mat[1].xyz
#define dirLightDiffuse(mat) mat[2].xyz
#define dirLightSpecular(mat) mat[3].xyz
// {
//     direction.x, direction.y, direction.z, _,
//     ambient.x,   ambient.y,   ambient.z,   _,
//     diffuse.x,   diffuse.y,   diffuse.z,   _,
//     specular.x,  specular.y,  specular.z,  _
// }
// ^ Matrix looks like this.

#define pointLightPos(mat) mat[0].xyz
#define pointLightAmbient(mat) mat[1].xyz
#define pointLightDiffuse(mat) mat[2].xyz
#define pointLightSpecular(mat) mat[3].xyz
#define pointLightConstant(mat) mat[1].w
#define pointLightLinear(mat) mat[2].w
#define pointLightQuadratic(mat) mat[3].w
// {
//     x,          y,          z,          _,
//     ambient.x,  ambient.y,  ambient.z,  constant,
//     diffuse.x,  diffuse.y,  diffuse.z,  linear,
//     specular.x, specular.y, specular.z, quadratic
// }
// ^ Matrix looks like this.

#define materialAmbient(mat) mat[0].xyz
#define materialDiffuse(mat) mat[1].xyz
#define materialSpecular(mat) mat[2].xyz
#define materialReflectivity(mat) mat[3].x
// {
//     ambient.x,    ambient.y,  ambient.z,  _,
//     diffuse.x,    diffuse.y,  diffuse.z,  _,
//     specular.x,   specular.y, specular.z, _,
//     reflectivity, _,          _,          _
// }
// ^ Matrix looks like this.

vec3 calcDirLight(mat4 material, mat4 dirLight, vec3 normalizedNormal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor)
{
    vec3 reflectDir = reflect(-dirLightNormNegDir(dirLight), normalizedNormal);
    float diffuseStrength = max(dot(normalizedNormal, dirLightNormNegDir(dirLight)), 0.0);
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), materialReflectivity(material));
    
    vec3 ambient = dirLightAmbient(dirLight) * materialDiffuse(material) * diffuseColor;
    vec3 diffuse = dirLightDiffuse(dirLight) * diffuseStrength * materialDiffuse(material) * diffuseColor;
    vec3 specular = dirLightSpecular(dirLight) * specularStrength * materialSpecular(material) * specularColor;
    return (ambient + diffuse + specular);
}
vec3 calcPointLight(mat4 material, mat4 pointLight, vec3 normalizedNormal, vec3 viewDir, vec3 fragPos, vec3 diffuseColor, vec3 specularColor)
{
    vec3 lightDir = normalize(pointLightPos(pointLight) - fragPos);
    vec3 reflectDir = reflect(-lightDir, normalizedNormal);
    float diffuseStrength = max(dot(normalizedNormal, lightDir), 0.0);
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), materialReflectivity(material));

    float dist = length(pointLightPos(pointLight) - fragPos);
    float attenuation = 1.0 / (pointLightConstant(pointLight) + pointLightLinear(pointLight) * dist + pointLightQuadratic(pointLight) * dist * dist);

    vec3 ambient = pointLightAmbient(pointLight) * materialDiffuse(material) * diffuseColor;
    vec3 diffuse = pointLightDiffuse(pointLight) * diffuseStrength * materialDiffuse(material) * diffuseColor;
    vec3 specular = pointLightSpecular(pointLight) * specularStrength * materialSpecular(material) * specularColor;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

#endif // LIGHTING_H_HEADER_GUARD