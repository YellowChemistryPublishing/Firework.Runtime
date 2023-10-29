#include "LitObject.h"
#include "GL.Config.h"

#include <cstring>
#include <iostream>

#include <GL/Lighting.h>
#include <GL/Renderer.h>
#include <GL/Shader.h>

using namespace Firework::Internal;
using namespace Firework::GL;

TextureSamplerHandle StaticLitObjectHandle::texColorSampler;

void StaticLitObjectHandle::submitDraw()
{
    Renderer::setDrawTransform(*this->internalTransform);
    Renderer::setDrawTexture(0, this->color, StaticLitObjectHandle::texColorSampler);

    Lighting::lightingProgram.setUniform("u_normalMatrix", this->internalTransform->tf.data);
    Lighting::lightingProgram.setUniform("u_material", this->internalMaterial->data);

    float dirLights[GL_MAX_DIRECTIONAL_LIGHT_COUNT][16]
    {{
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    }};
    
    {
        auto it = Lighting::dirLights.begin();
        for (size_t i = 0; i < GL_MAX_DIRECTIONAL_LIGHT_COUNT || it != Lighting::dirLights.end(); i++, ++it)
            memcpy(dirLights[i], it->data, sizeof(float[16]));
        Lighting::lightingProgram.setUniform("u_directionalLights", dirLights, GL_MAX_DIRECTIONAL_LIGHT_COUNT);
    }

    float pointLights[GL_MAX_PROXIMAL_POINT_LIGHT_COUNT][16]
    {{
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    }};

    {
        // TODO: Sort by closest point lights to object position.
        auto it = Lighting::pointLights.begin();
        for (size_t i = 0; i < GL_MAX_PROXIMAL_POINT_LIGHT_COUNT || it != Lighting::pointLights.end(); i++, ++it)
            memcpy(pointLights[i], it->data, sizeof(float[16]));
        Lighting::lightingProgram.setUniform("u_pointLights", pointLights, GL_MAX_PROXIMAL_POINT_LIGHT_COUNT);
    }

    Renderer::submitDraw(0, this->mesh, Lighting::lightingProgram);
}